/****************************  disasm1.cpp   ********************************
* Author:        Agner Fog
* Date created:  2007-02-25
* Last modified: 2007-06-01
* Project:       objconv
* Module:        disasm1.cpp
* Description:
* Module for disassembler.
*
* Most of the disassembler code is in this file.
* Instruction tables are in opcodes.cpp.
* All functions relating to file output are in disasm2.cpp
*
* (c) 2007 GNU General Public License www.gnu.org/copyleft/gpl.html
*****************************************************************************/
#include "stdafx.h"


/**************************  class CSymbolTable   *****************************

class CSymbolTable is a container class for a sorted list of symbols. The list
of symbols is kept sorted by address at all times. Named symbols from the 
original file are added to the list with AddSymbol(). New symbols for jump 
targets and code blocks that do not have a name are added during pass 1 by
NewSymbol(). AssignNames() assigns names to these unnamed symbols.

A symbol in the list can be found in three different ways: By its address,
by its old index, and by its new index. The new index is monotonous, so that
consecutive new indices correspond to consecutive addresses. Unfortunately,
the new index of a symbol will change whenever another symbol with a lower 
address is added to the list. Therefore, we need to use the old index rather
than the new index for identifying a symbol, e.g. in the relocation table.
The old index is a permanent, unique identifier, but in random order.
The old index of a symbol is usually the same as the index used in the
original file and in the relocation table. New symbols added during pass 1
will get assigned an old index which is higher than the highest value that
occurred in the original file. Do not make a pointer or reference to a symbol.
It may become invalid when new symbols are added.

To access a symbol by its old index, you have to translate it with Old2NewIndex
To access a symbol by its new index, use operator [].
To find a symbol by its address, use FindByAddress().

******************************************************************************/

CSymbolTable::CSymbolTable() {
   // Constructor
   OldNum = 1;
   NewNum = 0;                                   // Initialize
   UnnamedNum = 0;                               // Number of unnamed symbols
   UnnamedSymFormat = 0;                         // Format string for giving names to unnamed symbols
   UnnamedSymbolsPrefix = "?_";                  // Prefix to add to unnakes symbols
   ImportTablePrefix = "imp_";                   // Prefix for pointers in import table


   // Make dummy symbol number 0
   SASymbol sym0;
   sym0.Reset();
   sym0.Section = 0x80000000;                    // Lowest possible address
   List.PushSort(sym0);                          // Put into Symbols list

   SymbolNameBuffer.Push(0, 1);                  // Make string 0 empty
}

uint32 CSymbolTable::AddSymbol(int32 Section, uint32 Offset, uint32 Size, 
   uint32 Type, uint32 Scope, uint32 OldIndex, const char * Name, const char * DLLName) {
   // Add symbol from original file to symbol table.
   // If name is not known then set Name = 0. A name will then be assigned
   // OldIndex is the identifier used in relocation records. If the symbol is known
   // by address rather than by index, then set OldIndex = 0. The return value will
   // be the assigned value of OldIndex to use in relocation records. The returned value 
   // of OldIndex will be equal to the OldIndex of any previous symbols with same address.

   // Symbol record
   SASymbol NewSym;                              // New symbol table entry

   NewSym.Section  = Section;
   NewSym.Offset   = Offset;
   NewSym.Size     = Size;
   NewSym.Type     = Type;
   NewSym.Scope    = Scope;
   NewSym.OldIndex = OldIndex;

   // Store symbol name in NameBuffer
   if (Name && *Name) {
      NewSym.Name = SymbolNameBuffer.GetDataSize();
      if (DLLName) {
         // Imported from DLL. Prefix name with "imp_"
         SymbolNameBuffer.Push(ImportTablePrefix, (uint32)strlen(ImportTablePrefix));
      }
      // Store name
      SymbolNameBuffer.PushString(Name);
   }
   else {
      NewSym.Name = 0;                           // Will get a name later
   }
   // Store DLL name in NameBuffer
   if (DLLName && *DLLName) {
      NewSym.DLLName = SymbolNameBuffer.PushString(DLLName);
   }
   else {
      NewSym.DLLName = 0;
   }

   if (OldIndex == 0) {
      // Make non-unique entry
      uint32 NewIndex = NewSymbol(NewSym);
      // Get old index
      OldIndex = List[NewIndex].OldIndex;
   }
   else {
      // Make unique entry
      List.PushSort(NewSym);
   }

   // Set OldNum to 1 + maximum OldIndex
   if (OldIndex >= OldNum) OldNum = OldIndex + 1;

   return OldIndex;
}
 
uint32 CSymbolTable::NewSymbol(SASymbol & sym) {
   // Add symbol to symbol table.
   // Will not add a new symbol if one already exists at this address and
   // either the new symbol or the existing symbol has no name.
   // The return value is the new index to a new or existing symbol.
   // The type or scope of any existing symbol will be modified if
   // the type or scope of the new symbol is higher.
   // The name will be applied to the existing symbol if the existing symbol 
   // has no name.

   // Find new index of any existing symbol with same address
   int32 SIndex = FindByAddress(sym.Section, sym.Offset);

   if (SIndex > 0 && !(List[SIndex].Type & 0x80000000)
   && !(sym.Name && List[SIndex].Name)) {
      // Existing symbol found. Update it with type and scope

      // Choose between Type of existing symbol and new Type information.
      // The highest Type value takes precedence, except near indirect jump/call,
      // which has highest precedence
      if (((sym.Type & 0xFF) > (List[SIndex].Type & 0xFF)
      && (List[SIndex].Type+1 & 0xFE) != 0x0C) || (sym.Type+1 & 0xFE) == 0x0C) {
         // New symbol has higher type
         List[SIndex].Type = sym.Type;
      }
      if ((sym.Scope & 0xFF) > (List[SIndex].Scope & 0xFF)) {
         // New symbol has higher Scope
         List[SIndex].Scope = sym.Scope;
      }
      if (sym.Name && !List[SIndex].Name) {
         // New symbol has name, old symbol has no name
         List[SIndex].Name = sym.Name;
      }
   }
   else {
      // No existing symbol. Make new one
      // Give it an old index
      if (sym.OldIndex == 0) sym.OldIndex = OldNum++;

      SIndex = List.PushSort(sym);
   }

   // Return new index
   return SIndex;
}


uint32 CSymbolTable::NewSymbol(int32 Section, uint32 Offset, uint32 Scope) {
   // Add symbol to jump target or code block that doesn't have a name.
   // Will not add a new symbol if one already exists at this address.
   // The return value is the new index to a new or existing symbol.
   // The symbol will get a name later.

   // Symbol record
   SASymbol NewSym;                              // New symbol table entry
   NewSym.Reset();

   NewSym.Section  = Section;
   NewSym.Offset   = Offset;
   NewSym.Scope    = Scope;

   // Store new symbol record if no symbol with this address already exists
   return NewSymbol(NewSym);
}

void CSymbolTable::AssignNames() {
   // Assign names to symbols that do not have a name

   uint32 i;                                     // New symbol index
   char name[64];                                // Buffer for making symbol name

   // Format string
   if (NewNum < 1000) {
      UnnamedSymFormat = "%s%03i";
   }
   else {
      UnnamedSymFormat = "%s%04i";
   }

   // Update TranslateOldIndex
   UpdateIndex();

   // Loop through symbols
   for (i = 1; i < List.GetNumEntries(); i++) {
      if (List[i].Name == 0 && List[i].Scope != 0) {
         // Symbol has no name. Make one
         sprintf(name, UnnamedSymFormat, UnnamedSymbolsPrefix, ++UnnamedNum);
         // Store new name
         List[i].Name = SymbolNameBuffer.PushString(name);
      }
   }
   // Round up the value of UnnamedNum in case more names are assigned later
   if (NewNum < 1000) {
      UnnamedNum = (UnnamedNum + 199) / 100 * 100;
   }
   else {
      UnnamedNum = (UnnamedNum + 1999) / 1000 * 1000;
   }

#if 0
   // For debugging: list all symbols
   printf("\n\nSymbols:");
   for (i = 0; i < List.GetNumEntries(); i++) {

   //   if (List[i].Offset > 0x0 && List[i].Offset < 0x8)

      printf("\n%3X %3X %s Sect %i Offset %X Type %X Size %i Scope %i", 
         i, List[i].OldIndex, GetName(i), 
         List[i].Section, List[i].Offset, List[i].Type, List[i].Size, List[i].Scope);
   }
#endif
}

uint32 CSymbolTable::FindByAddress(int32 Section, uint32 Offset, uint32 * Last, uint32 * NextAfter) {
   // Find symbols by address
   // The return value will be the new index to the first symbol at the 
   // specified address. The return value will be zero if no symbol found.
   // If more than one symbol is found with the same address then Last
   // will receive the new index of the last symbol with this address.
   // NextAfter will receive the new index of the first symbol with an
   // address higher than the specified address in the same section, or 
   // zero if none.

   uint32 i1;                                    // New index of first symbol
   uint32 i2;                                    // New index of last symbol
   uint32 i3;                                    // New index of first symbol after address

   // Make dummy symbol record for searching
   SASymbol sym;
   sym.Section = Section;
   sym.Offset  = Offset;

   // Search List by address
   i1 = List.FindFirst(sym);

   if (i1 == 0 || i1 >= List.GetNumEntries()) {
      // No symbol found at this address or later. Return 0
      if (NextAfter) *NextAfter = 0;
      return 0;
   }
   if (sym < List[i1]) {
      // No symbol found at this address, but one found at higher address
      // Check if same section
      if (List[i1].Section != Section) i1 = 0;
      // Return symbol at later address
      if (NextAfter) *NextAfter = i1;
      return 0;
   }

   // A symbol was found at this address.
   // Search for more symbols at same address
   i2 = i1;
   while (i2+1 < List.GetNumEntries() && !(sym < List[i2+1])) i2++;

   // Search for first symbol after this address in same section
   if (i2+1 < List.GetNumEntries() && List[i2+1].Section == Section) {
      i3 = i2 + 1;                               // Found
   }
   else {
      i3 = 0;                                    // Not found
   }

   // Return last symbol at same address
   if (Last) *Last = i2;

   // Return first symbol at higher address
   if (NextAfter) *NextAfter = i3;

   // Return first symbol at address
   return i1;
}

uint32 CSymbolTable::FindByAddress(int32 Section, uint32 Offset) {
   // Find symbols by address
   // The return value will be the new index to a first symbol at the 
   // specified address. If more than one symbol is found at the same
   // address then the one with the highest scope (and which is not
   // a section record) is returned;
   uint32 s0, s1, s2 = 0;
   uint32 MaxScope = 0;
   // Find all symbols at this address
   s0 = s1 = FindByAddress(Section, Offset, &s2);
   // Check if any symbols found
   if (s0 == 0) return s0;

   // Loop through symbols at this address
   for (; s1 <= s2; s1++) {
      // Look for highest scope (and not section)
      if ((*this)[s1].Scope >= MaxScope && !((*this)[s1].Type & 0x80000000)) {
         s0 = s1;  MaxScope = (*this)[s1].Scope;
      }
   }
   // Return index to symbol with highest scope
   return s0;
}

uint32 CSymbolTable::Old2NewIndex(uint32 OldIndex) {
   // Translate old symbol index to new symbol index

   // Check if TranslateOldIndex is up to date
   if (NewNum != List.GetNumEntries()) {
      // New entries have been added since last update. Update TranslateOldIndex
      UpdateIndex();
   }
   // Check if valid
   if (OldIndex >= OldNum) OldIndex = 0;

   // Translate old index to new index
   uint32 NewIndex = TranslateOldIndex[OldIndex];

   // Check limit
   if (NewIndex >= NewNum) NewIndex = 0;

   // Return new index
   return NewIndex;
}

char * CSymbolTable::HasName(uint32 symo) {
   // Ask if symbol has a name, input = old index, output = name or 0
   // Returns 0 if symbol has no name yet.
   // Use HasName rather than GetName or GetNameO during pass 1 to avoid
   // naming symbols in random order.

   // Get new index
   uint32 symi = Old2NewIndex(symo);
   // Check if valid
   if (symi == 0 || symi >= NewNum) return 0;
   // Check if symbol has a name
   if ((*this)[symi].Name == 0) return 0;
   // Symbol has a name
   return GetName(symi);
}

char * CSymbolTable::GetName(uint32 symi) {
   // Get symbol name from new index.
   // A name will be assigned to the symbol if it doesn't have one

   if ((*this)[symi].Name == 0) {
      // Symbol has no name. Give symbol a name
      // (This should occur only if new symbols are made during pass 2)
      char name[64];                             // Buffer for making symbol name
      if (UnnamedSymFormat == 0) UnnamedSymFormat = "%s%03i";
      sprintf(name, UnnamedSymFormat, UnnamedSymbolsPrefix, ++UnnamedNum);
      // Store new name
      (*this)[symi].Name = SymbolNameBuffer.PushString(name);
   }

   // Get name index from symbol record
   uint32 NameIndex = (*this)[symi].Name;
   // Check if valid
   if (NameIndex == 0 || NameIndex >= SymbolNameBuffer.GetDataSize()) {
      // NameIndex is invalid
      return "ErrorNoName";
   }
   // Return name
   return SymbolNameBuffer.Buf() + NameIndex;
}

char * CSymbolTable::GetNameO(uint32 symo) {
   // Get symbol name by old index.
   // A name will be assigned to the symbol if it doesn't have one
   return GetName(Old2NewIndex(symo));
}

char * CSymbolTable::GetDLLName(uint32 symi) {
   // Get import DLL name from old index
   if ((*this)[symi].DLLName == 0) {
      // No name
      return "ErrorNoName";
   }
   // Get name DLL index from symbol record
   uint32 NameIndex = (*this)[symi].DLLName;
   // Check if valid
   if (NameIndex == 0 || NameIndex >= SymbolNameBuffer.GetDataSize()) {
      // NameIndex is invalid
      return "ErrorNoName";
   }
   // Return name
   return SymbolNameBuffer.Buf() + NameIndex;
}

void CSymbolTable::AssignName(uint32 symi, char *name) {
   // Give symbol a specific name
   (*this)[symi].Name = SymbolNameBuffer.PushString(name);
}

void CSymbolTable::UpdateIndex() {
   // Update TranslateOldIndex
   uint32 i;                                     // New index

   // Allocate array with sufficient size
   TranslateOldIndex.SetNum(OldNum);

   // Initialize to zeroes
   memset(&TranslateOldIndex[0], 0, TranslateOldIndex.GetNumEntries() * sizeof(uint32));

   for (i = 0; i < List.GetNumEntries(); i++) {
      if (List[i].OldIndex < OldNum) {
         TranslateOldIndex[List[i].OldIndex] = i;
      }
      else {
         // symbol index out of range
         err.submit(2031);                       // Report error
         List[i].OldIndex = 0;                   // Reset index that was out of range
      }
   }
   NewNum = List.GetNumEntries();
}


/**************************  class CDisassembler  *****************************


******************************************************************************/

CDisassembler::CDisassembler() {
   // Constructor
   CommentSeparator = "; ";                      // Use semicolon by default for indicating comments
   Sections.PushZero();                          // Make first section entry zero
   Relocations.PushZero();                       // Make first relocation entry zero
   NameBuffer.Push(0, 1);                        // Make first string entry zero   
   FunctionList.PushZero();                      // Make first function entry zero
   // Initialize variables
   Buffer = 0;
   InstructionSetMax = InstructionSetOR = FlagPrevious = 0;
   MasmOptions = RelocationsInSource = ExeType = 0;
   ImageBase = 0;
};

void CDisassembler::Init(uint32 ExeType, int64 ImageBase) {
   // Define file type and imagebase if executable file
   this->ExeType = ExeType;
   this->ImageBase = ImageBase;
}

void CDisassembler::AddSection(
   uint8 * Buffer,                               // Buffer containing raw data
   uint32  InitSize,                             // Size of initialized data in section
   uint32  TotalSize,                            // Size of initialized and uninitialized data in section
   uint32  SectionAddress,                       // Start address to be added to offset in listing
   uint32  Type,                                 // 0 = unknown, 1 = code, 2 = data, 3 = uninitialized data, 4 = constant data
   uint32  Align,                                // Alignment = 1 << Align
   uint32  WordSize,                             // Segment word size: 16, 32 or 64
   const char * Name,                            // Name of section
   uint32  NameLength) {                         // Length of name if not zero terminated

   // Check values
   if (Buffer == 0) Type = 3;
   if (Name == 0) Name = "?";
   if (NameLength == 0) NameLength = (uint32)strlen(Name);
   if (TotalSize < InitSize) TotalSize = InitSize;

   // Define section to be disassembled
   SASection SecRec;                             // New section record

   SecRec.Start = Buffer;
   SecRec.SectionAddress = SectionAddress;
   SecRec.InitSize = InitSize;
   SecRec.TotalSize = TotalSize;
   SecRec.Type = Type;
   SecRec.Align = Align;
   SecRec.WordSize = WordSize;
   // Save name in NameBuffer
   SecRec.Name = NameBuffer.Push(Name, NameLength);
   // Terminate with zero
   NameBuffer.Push(0, 1);
   // Default group is 'flat' except in 16 bit mode
   if (WordSize == 16 || (MasmOptions & 0x100)) {
      // 16-bit or mixed segment size. Group is unknown
      SecRec.Group = 0;
   }
   else {
      // Pure 32 or 64 bit mode. Group = flat
      SecRec.Group = ASM_SEGMENT_FLAT;
   }

   // Save section record
   Sections.Push(SecRec);

   // Remember WordSize
   switch (WordSize) {
   case 16:
      MasmOptions |= 0x100;  break;
   case 32:
      MasmOptions |= 0x200;  break;
   case 64:
      MasmOptions |= 0x400;  break;
   }
}

int32 CDisassembler::AddSectionGroup(const char * Name, int32 MemberSegment) {
   // Define section group (from OMF file).
   // Must be called after all segments have been defined.
   // To define a group with multiple members, you must call AddSectionGroup
   // multiple times. You must finish adding members to one group before 
   // starting the definition of another group.
   // You can define a group without defining its members by calling 
   // AddSectionGroup with MemberSegment = 0.

   // Check values
   if (Name == 0) Name = "?";

   // Find preceding segment or group definition
   int32 LastIndex = Sections.GetNumEntries() - 1;
   // Index of group record
   int32 GroupIndex = LastIndex;

   const char * LastName = "?";
   if (Sections[LastIndex].Name < NameBuffer.GetDataSize()) {
      // Last name valid
      LastName = NameBuffer.Buf() + Sections[LastIndex].Name;
   }
   // Check if group name already defined
   if (strcmp(Name, LastName) != 0) {
      // Not define. Make group record in Sections list
      SASection SecRec;                             // New section record
      memset(&SecRec, 0, sizeof(SecRec));           // Initialize

      // Set type = group
      SecRec.Type = 0x800;

      // Save name in NameBuffer
      SecRec.Name = NameBuffer.PushString(Name);

      // Save group index = my own index
      SecRec.Group = ++GroupIndex;

      // Save section record
      Sections.Push(SecRec);
   }
   // Find MemberSegment record
   if (MemberSegment && MemberSegment < GroupIndex) {
      // Register group index in segment record
      Sections[MemberSegment].Group = GroupIndex;
   }
   // Return value is group index
   return GroupIndex;
}

uint32 CDisassembler::AddSymbol(
   int32  Section,                            // Section number (1-based). ASM_SEGMENT_UNKNOWN = external, ASM_SEGMENT_ABSOLUTE = absolute, ASM_SEGMENT_IMGREL = image-relative
   uint32 Offset,                             // Offset into section. (Value for absolute symbol)
   uint32 Size,                               // Number of bytes used by symbol or function. 0 = unknown
   uint32 Type,                               // Symbol type. Use values listed above for SOpcodeDef operands. 0 = unknown type
   uint32 Scope,                              // 1 = function local, 2 = file local, 4 = public, 8 = weak public, 0x10 = communal, 0x20 = external
   uint32 OldIndex,                           // Unique identifier used in relocation entries. Value must be > 0 and limited because an array is created with this as index.
   const char * Name,                         // Name of symbol. Zero-terminated
   const char * DLLName) {                    // Name of DLL if imported dynamically

   // Add symbol form original file.
   // Multiple symbols at same address are allowed.
   // If section is not known then set Section = ASM_SEGMENT_IMGREL and Offset = image-relative address
   // If name is not known then set Name = 0. A name will then be assigned
   // OldIndex is the identifier used in relocation records. It must be nonzero.
   // If the original file uses 0-based symbol indices then add 1 to OldIndex
   // and remember to also add 1 when referring to the symbol in a relocation record.
   // If the symbol is known by address rather than by index, then set OldIndex = 0. 
   // The return value will be the assigned value of OldIndex to use in relocation records.
   // The returned value of OldIndex will be equal to the OldIndex of any previous symbols 
   // with same address. All symbols that have an identifier (OldIndex) must be defined
   // before any symbol identified by address only in order to avoid using the same OldIndex.

   // Check if image-relative
   if (Section == ASM_SEGMENT_IMGREL) {   
      // Translate absolute virtual address to section and offset
      TranslateAbsAddress(ImageBase + (int32)Offset, Section, Offset);
   }

   // Define symbol for disassembler
   return Symbols.AddSymbol(Section, Offset, Size, Type, Scope, OldIndex, Name, DLLName);
}

void CDisassembler::AddRelocation(
   int32  Section,                               // Section of relocation source
   uint32 Offset,                                // Offset of relocation source into section
   int32  Addend,                                // Addend to add to target address, 
                                                 // including distance from source to instruction pointer in self-relative addresses,
                                                 // not including inline addend.
   uint32 Type,                                  // 0 = unknown, 1 = direct, 2 = self-relative, 4 = image-relative, 
                                                 // 8 = segment relative, 0x10 = relative to arbitrary ref. point, 
                                                 // 0x100 = segment address/descriptor, 0x200 = segment of symbol
   uint32 Size,                                  // 1 = byte, 2 = word, 4 = dword, 8 = qword
   uint32 TargetIndex,                           // Symbol index of target
   uint32 ReferenceIndex) {                      // Symbol index of reference point if Type = 4 or 5

   // Check if image-relative
   if (Section == ASM_SEGMENT_IMGREL) {   
      // Translate absolute virtual address to section and offset
      TranslateAbsAddress(ImageBase + (int32)Offset, Section, Offset);
   }

   if (Type != 0x41) {
      // Define relocation or cross-reference for disassembler
      SARelocation RelRec;                          // New relocation record

      RelRec.Section = Section;
      RelRec.Offset = Offset;
      RelRec.Type = Type;
      RelRec.Size = Size;
      RelRec.Addend = Addend;
      RelRec.TargetOldIndex = TargetIndex;
      RelRec.RefOldIndex = ReferenceIndex;

      // Save relocation record
      Relocations.PushSort(RelRec);
   }
   else {
      // Make entry in procedure linkage table
      uint32 targetsym = Symbols.Old2NewIndex(TargetIndex);
      if (targetsym && Symbols[targetsym].DLLName) {
         // Put label on entry in procedure linkage table (import table)
         // Copy Name and DLLName from target symbol
         SASymbol ImportSym = Symbols[targetsym];
         ImportSym.Section = Section;
         ImportSym.Offset = Offset;
         ImportSym.Type = 0x0C;
         ImportSym.OldIndex = 0;
         ImportSym.Scope = 2;
         Symbols.NewSymbol(ImportSym);
      }
   }
}

void CDisassembler::Go() {
   // Do the disassembly

   // Check for illegal entries in relocations table
   InitialErrorCheck();

   // Find missing relocation target addresses
   FixRelocationTargetAddresses();

   // Pass 1: Find symbols types and unnamed symbols
   Pass = 1;
   Pass1();
   Pass = 2;
   Pass1(); 

   if (Pass & 0x100) {
      // Repetition of pass 1 requested
      Pass = 3;
      Pass1();
      Pass = 4;
      Pass1();
   }

   // Put names on unnamed symbols
   Symbols.AssignNames();

   // Fix invalid characters in symbol and section names
   CheckNamesValid();

#if 0
   // Show function list. For debugging only
   printf("\n\nFunctionList:");
   for (uint32 i = 0; i < FunctionList.GetNumEntries(); i++) {
      printf("\nsect %i, start %X, end %X, scope %i, name %s",
         FunctionList[i].Section, FunctionList[i].Start, FunctionList[i].End, 
         FunctionList[i].Scope, Symbols.GetNameO(FunctionList[i].OldSymbolIndex));
   }
#endif
#if 0
   // For debugging: list all relocations
   printf("\n\nRelocations:");
   for (uint32 i = 0; i < Relocations.GetNumEntries(); i++) {
      printf("\nsect %i, os %X, type %X, size %i, add %X, target %X",
         Relocations[i].Section, Relocations[i].Offset, Relocations[i].Type, 
         Relocations[i].Size, Relocations[i].Addend, Relocations[i].TargetOldIndex);
   }
#endif
#if 0
   // For debugging: list all sections
   printf("\n\nSections:");
   for (uint32 s = 1; s < Sections.GetNumEntries(); s++) {
      printf("\n%2i, %s", s, NameBuffer.Buf() + Sections[s].Name);
   }
#endif

   // Begin writing output file
   WriteFileBegin();

   // Pass 2: Write all sections to output file
   Pass = 0x10;
   Pass2();

   // Check for illegal entries in symbol table and relocations table
   FinalErrorCheck();

   // Finish writing output file
   WriteFileEnd();
};

void CDisassembler::Pass1() {

   /*             Pass 1: does the following jobs:
                  --------------------------------
   
   * Scans all code sections, instruction by instruction. Checks code syntax.

   * Tries to identify where each function begins and ends.

   * Follows all references to data in order to determine data type for 
     each data symbol.

   * Assigns symbol table entries for all jump and call targets that do not
     allready have a name.

   * Follows all jump instructions to identify code blocks that are connected.
     Code blocks in same section that are connected through jumps (not calls)
     are joined together into the same function.

   * Identifies and analyzes tables of jump addresses and call addresses,
     e.g. switch/case tables and virtual function tables.

   * Tries to identify any data in the code section. If erroneous code or
     sequences of zeroes are found then the nearest preceding label is marked
     as dubious and the analysis of code is skipped until the next code label.
     Pass 1 will be repeated in this case in order to follow backwards jumps
     from subsequent code. Dubious code will be shown as both code and data 
     in the output of pass 2.
   */

   // Loop through sections, pass 1
   for (Section = 1; Section < Sections.GetNumEntries(); Section++) {

      // Get section type
      SectionType = Sections[Section].Type;
      if (SectionType & 0x800) continue;         // This is a group

      // Code or data
      CodeMode = (SectionType & 1) ? 1 : 4;
      LabelBegin = FlagPrevious = CountErrors = 0;

      if ((Sections[Section].Type & 0xFF) == 1) {
         // This is a code section

         // Initialize code parser
         Buffer     = Sections[Section].Start;
         SectionEnd = FunctionEnd = LabelInaccessible = Sections[Section].TotalSize;
         WordSize   = Sections[Section].WordSize;
         SectionAddress = Sections[Section].SectionAddress;
         if (Buffer == 0) continue;

         IBegin = IEnd = LabelEnd = 0;
         IFunction = 0;

         // Loop through instructions
         while (NextInstruction1()) {

            // check if function beings here
            CheckForFunctionBegin();

            // Find any label here
            FindLabels();

            // Check if code
            if (CodeMode < 4) {
               // This is code

               // Parse instruction
               ParseInstruction();
            }
            else {
               // This is data. Skip to next label
               IEnd = LabelEnd;
            }
            // check if function ends here
            CheckForFunctionEnd();
         }
      }
      else {
         // This is a data section
         // Make a single entry in FunctionList covering the whole section
         SFunctionRecord fun = {Section, 0, Sections[Section].TotalSize, 0, 0};
         FunctionList.PushUnique(fun);
      }
   }
}

void CDisassembler::FindLabels() {
   // Find any labels at current position and next during pass 1
   uint32 sym1, sym2 = 0, sym3 = 0;              // Symbol indices

   // Search for labels from IBegin
   sym1 = Symbols.FindByAddress(Section, IBegin, &sym2, &sym3);

   if (sym1 && sym2) {
      // Set LabelBegin to address of last label at current address
      LabelBegin = Symbols[sym2].Offset;
      CountErrors = 0;

      // Get code mode from label
      if ((Symbols[sym2].Type & 0xF0) == 0x80) {
         // This is known to be code
         CodeMode = 1;
      }
      else if ((Symbols[sym2].Type & 0xFF) == 0) {
         ; // Type is unknown. Assume same type as previous
      }
      else {
         // This is known to be data
         CodeMode = 4;
      }
      // Reset tracer
      t.Reset();
   }
   if (sym3) {
      // Set LabelEnd to address of next symbol
      LabelEnd = Symbols[sym3].Offset;
      if (LabelEnd > SectionEnd) LabelEnd = SectionEnd;
   }
   else {
      // No next label
      LabelEnd = SectionEnd;
   }
}

int CDisassembler::NextLabel() {
   // Loop through labels from IEnd. Pass 2
   uint32 sym, sym1, sym2 = 0, sym3 = 0;         // Symbol indices

   // Make ready for next instruction
   IBegin = IEnd;

   // Reset tracer
   t.Reset();

   // Check if end of function/section
   if (IEnd >= FunctionEnd || IEnd >= SectionEnd) {
      // No more labels in this function or section
      return 0;
   }

   // Search for labels from IEnd
   sym1 = Symbols.FindByAddress(Section, IEnd, &sym2, &sym3);

   if (sym1) {
      // Symbol found
      for (sym = sym1; sym <= sym2; sym++) {
         // Remember symbol address
         LabelBegin = Symbols[sym].Offset;
         CountErrors = 0;

         if ((SectionType & 0xFF) == 1) {
            // Code section. Get CodeMode

            if ((Symbols[sym].Type >> 24) & 0xF) {
               // Get CodeMode from last label. 1 = code, 2 = dubiuos, 4 = data
               CodeMode = (Symbols[sym].Type >> 24) & 0xF;
            }
            else if (Symbols[sym].Type & 0x80) {
               // Type defined as jump/call. This is known to be code
               CodeMode = 1;
            }
            else if (Symbols[sym].Type == 0) {
               // Type is unknown. (Assume same type as previous) changed to:
               // Type is unknown. Assume code
               CodeMode = 1;
            }
            else {
               // This has been accessed as data
               CodeMode = 4;
            }
         }
         else {
            // This is a data segment
            CodeMode = 4;
         }
         // Get symbol type and size, except for section type
         if (!(Symbols[sym].Type & 0x80000000)) {
            DataType = Symbols[sym].Type;
            DataSize = GetDataItemSize(DataType);
            if ((DataType+1 & 0xFE) == 0x0C && Symbols[sym].Size) {
               // Jump table can have different sizes for direct or image relative
               DataSize = Symbols[sym].Size;
            }
         }
      }
   }
   if (sym3) {
      // Next label found
      LabelEnd = Symbols[sym3].Offset;
      return 1;
   }
   // No new label found. Continue to FunctionEnd
   LabelEnd = FunctionEnd;
   return 1;
}

int CDisassembler::NextFunction2() {
   // Loop through function blocks in pass 2. Return 0 if finished

   SFunctionRecord Fun;                          // Dummy function record for search and compare

   if (IFunction == 0) {
      // Begin of section. Find first function block
      Fun.Section = Section;
      Fun.Start   = IBegin;
      IFunction   = FunctionList.FindFirst(Fun);
   }
   else {
      // Try next function block
      IFunction++;
   }
   // Check if IFunction is valid
   if (IFunction == 0 || IFunction >= FunctionList.GetNumEntries()) {
      // Not valid
      IFunction = 0;
      return 0;
   }
   // Check if IFunction is within current section
   Fun.Section = Section;
   Fun.Start   = SectionEnd;
   if (Fun < FunctionList[IFunction]) {
      // Past end of current section
      IFunction = 0;
      return 0;
   }
   // IFunction is within current section
   // End of function
   FunctionEnd = FunctionList[IFunction].End;

   // Check if function has a defined size
   if (FunctionEnd <= FunctionList[IFunction].Start) {
      // Size unknown. Continue until begin of next function
      if (IFunction+1 < FunctionList.GetNumEntries()
      && FunctionList[IFunction+1] < Fun
      && FunctionList[IFunction] < FunctionList[IFunction+1]) {
         FunctionEnd = FunctionList[IFunction+1].Start;
      }
      else {
         // No next function. Continue until end of section
         FunctionEnd = SectionEnd;
      }
   }

   // return IFunction for success
   return 1;
}

void CDisassembler::CheckForFunctionBegin() {
   // Check if function begins at current position
   uint32 sym1, sym2 = 0, sym3 = 0;              // Symbol indices
   SFunctionRecord fun;                          // New function record
   IBegin = IEnd;

   if (IFunction == 0) {
      // No function defined. Begin new function here

      // Search for nearest labels
      sym1 = Symbols.FindByAddress(Section, IEnd, &sym2, &sym3);

      if (sym1 == 0) {
         // There is no label here. Make one with Scope = 0
         sym1 = Symbols.NewSymbol(Section, IEnd, 0);
         // Update labels
         LabelBegin = LabelEnd = CountErrors = 0;
         FindLabels();
      }
      // Check that sym1 is valid
      if (sym1 == 0 || sym1 >= Symbols.GetNumEntries()) {
         err.submit(9000);  return;
      }

      // Make function record for FunctionList
      fun.Section        = Section;
      fun.Start          = IBegin;
      fun.End            = IBegin;
      fun.Scope          = Symbols[sym1].Scope;
      fun.OldSymbolIndex = Symbols[sym1].OldIndex;

      // Add to function list
      IFunction = FunctionList.PushUnique(fun);

      // End of function not known yet
      FunctionEnd = SectionEnd;  LabelEnd = 0;
   }
}

void CDisassembler::CheckForFunctionEnd() {
   // Check if function ends at current position
   if (IFunction >= FunctionList.GetNumEntries()) {
      // Should not occur
      err.submit(9000);  IFunction = 0;  return;
   }

   // Function ends if section ends here or if last instruction was a return
   if (IEnd >= SectionEnd) {
      // Current function must end because section ends here
      FunctionList[IFunction].End = IEnd;

      // Check if return instruction
      if (s.OpcodeDef && !(s.OpcodeDef->Options & 0x10) && (Pass & 0x10)) {
         // No return or unconditional jump. Write error message
         s.Errors |= 0x10000;
         WriteErrorsAndWarnings();
      }
      // Indicate current function ends here
      FunctionList[IFunction].End = IEnd;
      IFunction = 0;
      return;
   }

   // Function ends after ret or unconditional jump and preceding code had no 
   // jumps beyond this position:
   if (s.OpcodeDef && s.OpcodeDef->Options & 0x10) {
      // A return or unconditional jump instruction was found.
      FlagPrevious |= 2;

      // Mark this position as inaccessible if there is no reference to this place
      Symbols.NewSymbol(Section, IEnd, 0);
      // Update labels
      LabelBegin = LabelEnd = CountErrors = 0;
      FindLabels();

      if (IEnd >= FunctionList[IFunction].End) {
         // Indicate current function ends here
         FunctionList[IFunction].End = IEnd;
         IFunction = 0;
         return;
      }
   }

   // Function ends at next label if preceding label is inaccessible 
   if (IFunction && FunctionList[IFunction].Scope == 0 && IEnd >= FunctionList[IFunction].End) {
      if (Symbols.FindByAddress(Section, IEnd)) {
         // Previous label was inaccessible. There is a new label here. Begin new function here
         IFunction = 0;
         return;
      }
   }

   // Function does not end here
   return;
}

void CDisassembler::CheckRelocationTarget(uint32 IRel, uint32 TargetType, uint32 TargetSize) {
   // Update relocation record and its target.
   // This function updates the symbol type and size of a relocation target.
   // If the relocation target is a section:offset address then a new
   // symbol record is made
   uint32 SymOldI;                               // Old index of target symbol
   uint32 SymNewI;                               // New index of target symbol
   int32  TargetSection;                         // Section of target symbol
   uint32 TargetOffset;                          // Offset of target symbol

   // Check if relocation valid
   if (!IRel || IRel >= Relocations.GetNumEntries() || !Relocations[IRel].TargetOldIndex
   || Relocations[IRel].Section <= 0 || uint32(Relocations[IRel].Section) >= Sections.GetNumEntries()) {
      return;
   }

   // Find target symbol
   SymOldI = Relocations[IRel].TargetOldIndex;

   // Look up in symbol table
   SymNewI = Symbols.Old2NewIndex(SymOldI);

   // Check if valid
   if (!Symbols[SymNewI].OldIndex) return;

   if (Symbols[SymNewI].Type & 0x80000000) {
      // Symbol is a section record. Relocation refers to a section-relative address
      // Make a new symbol for this data item. The symbol will get a name later

      // Get address of new symbol
      TargetSection = Symbols[SymNewI].Section;
      TargetOffset  = Symbols[SymNewI].Offset + Relocations[IRel].Addend;

      // Pointer to relocation source address
      uint8 * RelSource = Sections[Relocations[IRel].Section].Start + Relocations[IRel].Offset;

      // Inline Addend;
      int32 InlineA = 0;
      switch (Relocations[IRel].Size) {
      case 1:
         InlineA = *(int8*)RelSource;  break;
      case 2:
         InlineA = *(int16*)RelSource;  break;
      case 4:  case 8:
         InlineA = *(int32*)RelSource;  break;
      }
      // Add inline addend to target address
      TargetOffset += InlineA;

      if (Relocations[IRel].Type & 2) {
         // Address is self-relative
         if (s.AddressFieldSize && (s.MFlags & 0x100) || s.ImmediateFieldSize) {
            // Relative jump or rip-relative address
            TargetOffset += IEnd - s.AddressField;
            InlineA      -= IEnd - s.AddressField;
         } 
         else {
            // Self-relative address in data segment or unknown (extremely rare)
            TargetOffset += Relocations[IRel].Size;
            InlineA      -= Relocations[IRel].Size;
         }
      }
      // Make new symbol in symbol table if none exists
      SymNewI = Symbols.NewSymbol(TargetSection, TargetOffset, 2);

      if (SymNewI) {
         // Get old index
         SymOldI = Symbols[SymNewI].OldIndex;

         // Change relocation record to point to new symbol
         Relocations[IRel].TargetOldIndex = SymOldI;

         // Compensate for inline addend and rip-relative address
         Relocations[IRel].Addend = -InlineA;
      }
   }

   // Check if symbol has a scope assigned
   if (Symbols[SymNewI].Scope == 0) Symbols[SymNewI].Scope = 2;

   // Choose between Symbols[SymNewI].Type and TargetType the one that has the highest priority
   if ((TargetType & 0xFF) > (Symbols[SymNewI].Type & 0xFF) 
   || (TargetType+1 & 0xFE) == 0x0C && (Symbols[SymNewI].Type & 0xFF) > 0x0C) {

      // No type assigned yet, or new type overrides old type
      Symbols[SymNewI].Type = TargetType;

      // Choose biggest size. Size for code pointer takes precedence
      if (TargetSize > Symbols[SymNewI].Size || (TargetType+1 & 0xFE) == 0x0C) {
         Symbols[SymNewI].Size = TargetSize;
      }
   }
}

void CDisassembler::CheckJumpTarget(uint32 symi) {
   // Extend range of current function to jump target, if needed

   // Check if current section is valid
   if (Section == 0 || Section >= Sections.GetNumEntries()) return;

   // Check if current function is valid
   if (IFunction == 0 || IFunction >= FunctionList.GetNumEntries()) return;

   // Check if target is in same section
   if (Symbols[symi].Section != Section) return;

   // Check if target extends current function
   if (Symbols[symi].Offset > FunctionList[IFunction].End && Symbols[symi].Offset <= Sections[Section].InitSize) {
      // Target is after tentative end of current function but within section

      // Check if it is a known function
      if ((Symbols[symi].Type & 0xFF) == 0x83 || (Symbols[symi].Type & 0xFF) == 0x85
      || (Symbols[symi].Scope & 0x1C)) {
         // Target is known as public or a function. No need to extend current function
         return;
      }
      // Extend current function forward to include at least one instruction after target offset
      FunctionList[IFunction].End = Symbols[symi].Offset + 1;
   }
   else if (Symbols[symi].Offset < FunctionList[IFunction].Start) {
      // Target is before tentative begin of current function but within section

      // Check if target is already in function table
      SFunctionRecord fun;
      fun.Section = Symbols[symi].Section;
      fun.Start   = Symbols[symi].Offset;
      uint32 IFun = FunctionList.Exists(fun);
      if (IFun > 0 && IFun < FunctionList.GetNumEntries()) {
         // Target is the beginning of a known function. No need to extend current function
         return;
      }
      // Target points inside a previously defined function. Join the two functions into one
      IFun = FunctionList.FindFirst(fun) - 1;
      if (IFun > 0 && IFun < FunctionList.GetNumEntries() && FunctionList[IFun].Section == Section) {

         // Get maximum scope of the two functions
         if (FunctionList[IFun].Scope < FunctionList[IFunction].Scope) {
            FunctionList[IFun].Scope = FunctionList[IFunction].Scope;
         }

         // Get maximum end of the two functions
         if (FunctionList[IFun].End < FunctionList[IFunction].End) {
            FunctionList[IFun].End = FunctionList[IFunction].End;
         }         

         // Remove entry IFunction from FunctionList
         FunctionList.Remove(IFunction);

         // Set current function to IFun
         IFunction = IFun;
      }
   }
}

void CDisassembler::Pass2() {

   /*             Pass 2: does the following jobs:
                  --------------------------------
   
   * Scans through all sections, code and data.

   * Code is analyzed, instruction by instruction. Checks code syntax.

   * Outputs warnings for suboptimal instruction codes and error messages
     for erroneous code and erroneous relocations.

   * Outputs disassembly of all instructions, operands and relocations, 
     followed by the binary code listing as comment.

   * Outputs disassembly of all data, followed by alternative representations
     as comment.

   * Outputs dubious code as both code and data in order to allow a re-assembly
     to produce identical code.
   */

   // Loop through sections, pass 2
   for (Section = 1; Section < Sections.GetNumEntries(); Section++) {

      // Get section type
      SectionType = Sections[Section].Type;
      if (SectionType & 0x800) continue;         // This is a group

      if (((SectionType & 0xFF) == 0x10) && cmd.DebugInfo == CMDL_DEBUG_STRIP) {
         // Skip debug section
         cmd.CountDebugRemoved();
         continue;
      }
      if (((SectionType & 0xFF) == 0x11) && cmd.ExeptionInfo == CMDL_EXCEPTION_STRIP) {
         // Skip exception section
         cmd.CountExceptionRemoved();
         continue;
      }
      // Is this code or data?
      CodeMode = ((SectionType & 0xFF) == 1) ? 1 : 4;

      // Initialize
      LabelBegin = FlagPrevious = CountErrors = 0;
      Buffer = Sections[Section].Start;
      SectionEnd = Sections[Section].TotalSize;
      LabelInaccessible = Sections[Section].InitSize;
      WordSize = Sections[Section].WordSize;
      SectionAddress = Sections[Section].SectionAddress;

      // Write segment directive
      WriteSegmentBegin();

      IBegin = IEnd = LabelEnd = IFunction = DataType = DataSize = 0;

      // Loop through function blocks in this section
      while (NextFunction2()) {

         // Write begin function
         if (CodeMode & 3) WriteFunctionBegin();

         // Loop through labels
         while (NextLabel()) {

            // Loop through code
            while (NextInstruction2()) {

               if (CodeMode & 3) {
                  // Interpret this as code

                  // Write label if any
                  CheckLabel();

                  // Parse instruction
                  ParseInstruction();

                  // Check for filling space
                  if (((s.Warnings & 0x10000000) || s.Warnings == 0x1000000) && WriteFillers()) {
                     // Code is inaccessible fillers. Has been written by CheckForFillers()
                     continue;
                  }

                  // Write any error and warning messages to OutFile
                  WriteErrorsAndWarnings();

                  // Write instruction to OutFile
                  WriteInstruction();

                  // Write hex code as comment after instruction
                  WriteCodeComment();
               }
               if (CodeMode & 6) {

                  // Interpret this as data
                  WriteDataItems();
               }
               if (IEnd <= IBegin) {

                  // Prevent infinite loop
                  IEnd++;
                  break;
               }
            }
         }
         // Write end of function, if any
         if (CodeMode & 3) WriteFunctionEnd();         // End function
      }
      // Write end of segment
      WriteSegmentEnd();
   }
}

/********************  Explanation of tracer:  ***************************

This is a machine which can trace the contents of each register in certain
situations. It is currently used for recognizing certain instruction patterns
that are used by MS and Intel 64 bit compilers for accessing jump tables and
virtual function tables. The trace machine can be extended for other purposes.

A switch/case statement is typically implemented as follows by the 64 bit MS
C++ compiler:

.code
        lea     rbx, [__ImageBase]
        mov     eax, [SwitchIndex]
        add     eax, - LowerLimit
        cmp     eax, Range
        ja      LabelDefault
        cdqe
        mov     ecx, [imagerel(SwitchTable) + rbx + rax*4]
        add     rcx, rbx
        jmp     rcx

.data
SwitchTable label dword
        dd      imagerel(Label1)
        dd      imagerel(Label2)
        dd      imagerel(Label3)

In order to recognize that SwitchTable is a switch table, the tracer must
do the following tasks:

1.  Calculate the rip-relative address in the lea instruction and detect
    that it is equal to the image base. 
    
2.  Remember that rbx contains the image base.

3.  When interpreting the mov ecx instruction it recognizes that the base
    pointer contains the image base, therefore the displacement must be
    interpreted as an image-relative address. Calculate this address and
    give it a name.

4.  Remember that ecx contains an an element from the array SwitchTable.
    It is not yet known that SwitchTable is a switch table.

5.  After add rcx,rbx remember that rcx contains an element from the array
    SwitchTable plus the image base.

6.  When interpreting the jmp rcx instruction, the information about the 
    contents of rcx is used for concluding that SwitchTable contains jump
    addresses, and that these addresses are image-relative. If there had
    been no add rcx,rbx, we would conclude that SwitchTable contains
    absolute virtual addresses.

7.  Go through all elements of SwitchTable. Calculate the address that each
    element points to, give it a name, and extend the scope of the current
    function to include this target.

8.  It would be possible to determine the length of the switch table from
    the cmp instruction, but the tracer does not currently use this 
    information. Instead, it stops parsing the switch table at the first 
    known label or the first invalid address.

This is quite a long way to go for acquiring this information, but it is 
necessary in order to tell what is code and what is data and to find out
where the function ends. Unfortunately, the MS compiler puts switch tables
in the code segment rather than in the data segment which would give better
caching and code prefetching. If the switch table was not identified as such,
it would be impossible to tell what is code and what is data.

The tracer is also used for identifying virtual function tables.

Values of SATracer::Regist[i] tells what kind of information register i contains:
   0     Unknown contents
   1     Contains image base
   4     Contains a constant = Value[i]
   8     Contains a value < Value[i]. (Not implemented yet)
   0x10  Contains the value of a symbol. Value[i] contains the old index of the symbol
   0x11  Contains the value of an array element. Value[i] contains the symbol old index of the array
   0x12  Contains the value of an array element + image base. Value[i] contains the symbol old index of the array
   0x18  Contains the address of a symbol. Value[i] contains the symbol old index
   0x19  Contains the address of an array element. Value[i] contains the symbol old index of the array
*/

void CDisassembler::UpdateTracer() {
   // Trace register values. See explanation above
   uint32 reg;                                   // Destination register number
   uint32 srcreg;                                // Source register number

   if (s.Operands[0] & 0xFF) {
      // There is a destination operand
      if ((s.Operands[0] & 0xFF) < 5 && !(s.Operands[0] & 0x300)) {
         // Destination operand is a general purpose register
         if (s.Operands[0] & 0x10000) {
            // Register indicated by reg bits of mod/reg/rm byte
            reg = s.Reg;
         }
         else if (s.Operands[0] & 0x20000) {
            if ((s.OpcodeDef->InstructionFormat & 0x1F) == 3) {
               // Register indicated by last bits of opcode byte
               reg = Get<uint8>(s.OpcodeStart2) & 7;
               // Check REX.B prefix
               if (s.Prefixes[7] & 1) reg |= 8;     // Add 8 if REX.B prefix
            }
            else {
               // Register indicated by rm bits of mod/reg/rm byte
               reg = s.RM;
            }
         }
         else {
            // Error. Don't know where to find destination register
            t.Reset();  return;
         }
      }
      else if ((s.Operands[0] & 0xFF) >= 0xA0 && (s.Operands[0] & 0xFF) <= 0xA9) {
         // Destination is al, ax, eax, or rax
         reg = 0;
      }
      else {
         // Destination is not a general purpose register
         return;
      }
   }
   else {
      // There is no destination operand
      return;
   }

   // Destination operand is a general purpose register
   if (OpcodeOptions & 4) {
      // Destination register is not changed
      return;
   }

   // Check the opcode to find out what has happened to this register
   switch (Opcodei) {
   /* This part is currently unused:
   case 0xB0: case 0xB1: case 0xB2: case 0xB3: 
   case 0xB4: case 0xB5: case 0xB6: case 0xB7:
   case 0xB8: case 0xB9: case 0xBA: case 0xBB: 
   case 0xBC: case 0xBD: case 0xBE: case 0xBF:
      // MOV register, constant
      t.Regist[reg] = 0;
      if (s.OperandSize < 32) {
         // Only part of register is changed
         return;
      }
      if (s.ImmediateRelocation) {
         if (s.OperandSize < WordSize || !(Relocations[s.ImmediateRelocation].Type & 0x21)) {
            // Wrong size or type of relocation
            return;
         }
         // Register contains the address of a symbol
         t.Regist[reg] = 0x18;
         t.Value [reg] = Relocations[s.ImmediateRelocation].TargetOldIndex;
         return;
      }

      // Register value is a known constant
      t.Regist[reg] = 4;
      // Save value
      switch (s.ImmediateFieldSize) {
      case 1:
         t.Value[reg] = Get<uint8>(s.ImmediateField);
         break;
      case 2:
         t.Value[reg] = Get<uint16>(s.ImmediateField);
         break;
      case 4: 
      case 8: // 64-bit value truncated to 32 bits
         t.Value[reg] = Get<uint32>(s.ImmediateField);
         break;
      default:
         // Error. Should not occur
         t.Regist[reg] = 0;
      }
      return;
   case 0x31: case 0x33: case 0x29: case 0x2B:
      // XOR or SUB. Check if source and destination is same register
      if ((s.Operands[0] & 0xFFF) == (s.Operands[1] & 0xFFF) && s.Reg == s.RM && s.OperandSize >= 32) {
         // XOR OR SUB with same source and destination produces zero
         t.Regist[reg] = 4;
         t.Value [reg] = 0;
         return;
      }
      break;
   */

   case 0x8D:
      // LEA
      if (s.AddressFieldSize == 4 && s.AddressRelocation && s.OperandSize >= 32) {
         // Register contains the address of a symbol
         t.Regist[reg] = 0x18;
         t.Value [reg] = Relocations[s.AddressRelocation].TargetOldIndex;
         // Check if symbol has name
         const char * SymName = Symbols.HasName(t.Value[reg]);
         if (SymName && strcmp(SymName, "__ImageBase") == 0) {
            // Symbol is imagebase
            t.Regist[reg] = 1;
         }
         // Check if base or index register
         if (s.BaseReg || s.IndexReg) t.Regist[reg]++;
         return;
      }
      break;

   case 0x89: case 0x8B:
      // MOV instruction
      if (s.OperandSize < 32) break;          // Only part of register is changed
      if (!(s.MFlags & 1)) {
         // MOV reg,reg. Copy register contents
         if (Opcodei == 0x8B) {
            // Source register indicated by rm bits
            srcreg = s.RM;
         }
         else {
            // Source register indicated by reg bits
            srcreg = s.Reg;
         }
         t.Regist[reg] = t.Regist[srcreg];
         t.Value [reg] = t.Value [srcreg];
         return;
      }
      // MOV reg,mem
      if (s.AddressFieldSize == 4 && s.AddressRelocation) {
         // Register contains the value of a symbol
         t.Regist[reg] = 0x10;
         t.Value [reg] = Relocations[s.AddressRelocation].TargetOldIndex;

         // Check if base or index register
         if (s.BaseReg || s.IndexReg) t.Regist[reg]++;
         return;
      }
      if (s.BaseReg && (t.Regist[s.BaseReg-1] & 0xFE) == 0x18) {
         // Memory operand has a base register which contains the address of a symbol
         // Destination register will contain value of same symbol
         t.Regist[reg] = 0x10;
         t.Value [reg] = t.Value[s.BaseReg-1];
         if (s.IndexReg || s.AddressFieldSize || (t.Regist[s.BaseReg-1] & 1)) {
            // There is an offset
            t.Regist[reg] |= 1;
         }
         return;
      }
      break;
   case 0x01: case 0x03:
      // ADD instruction
      if (s.OperandSize < 32) break;          // Only part of register is changed
      if (Opcodei == 0x03) {
         // Source register indicated by rm bits
         srcreg = s.RM;
      }
      else {
         // Source register indicated by reg bits
         srcreg = s.Reg;
      }
      if (t.Regist[srcreg] == 1 && (t.Regist[reg] & 0xFE) == 0x10) {
         // Adding imagebase to the value of a symbol or array element
         t.Regist[reg] = 0x12;
         return;
      }
      if (t.Regist[reg] == 1 && (t.Regist[srcreg] & 0xFE) == 0x10) {
         // Adding the value of a symbol or array element to the imagebase
         t.Regist[reg] = 0x12;
         t.Value [reg] = t.Value[srcreg];
         return;
      }
      break;
   case 0x3902:
      // CDQE. eax sign extended to rax. Ignore
      return;
   case 0x3900: case 0x3901:
      // CBW, CWDE. rax changed
      t.Regist[0] = 0;
      return;
   case 0x3A00: case 0x3A01: case 0x3A02:
      // CWD, CDQ, CQO. rdx changed
      t.Regist[2] = 0;
      return;
   }
   // Anything else: Remember that this register is changed
   t.Regist[reg] = 0;

   if (OpcodeOptions & 8) {
      // Registers other than destination register may be changed
      t.Reset();
   }
}

void CDisassembler::UpdateSymbols() {
   // Find unnamed symbols, determine symbol types,
   // update symbol list, call CheckJumpTarget if jump/call.
   // This function is called during pass 1 for every instruction

   uint32 OpI;                                   // Operand index
   uint32 OperandType;                           // Type of operand
   uint32 SymOldI;                               // Symbol table old index
   uint32 SymNewI;                               // Symbol table new index

   // Loop through all operands for one instruction
   for (OpI = 0; OpI < 3; OpI++) {
      if (s.Operands[OpI]) {
         SymNewI = 0;                            // Reset symbol index
         OperandType = s.Operands[OpI];          // Operand type

         // Check if indirect jump/call
         if (OpI == 0 && (s.OpcodeDef->Destination + 1 & 0xFE) == 0x0C) {
            OperandType = s.OpcodeDef->Destination;
         }

         // Check operand type
         if ((OperandType & 0xF0) == 0x80) {
            // This is a jump/call destination

            if (!s.ImmediateRelocation) {
               // Has no reference to other symbol. Make one

               // Relocation type
               uint32 RelocationType = 2;        // Self relative
               if ((OperandType & 0xFE) == 0x84) RelocationType = 8; // Far

               // Scope
               uint32 TargetScope = 1;           // Function local    
               if ((OperandType & 0xFF) >= 0x83) TargetScope = 2;  // Call or far. File scope

               // Make relocation and target symbol
               SymNewI = MakeMissingRelocation(Section, s.ImmediateField, RelocationType, OperandType, TargetScope);

               // Update labels
               LabelBegin = 0;
               FindLabels();

               if (TargetScope == 1 && SymNewI) {
                  // Short or near jump (not call). Update range of current function
                  CheckJumpTarget(SymNewI);
               }
            }
            else {
               // Jump or call to relocated symbol
               // Look up in Relocations table
               SymOldI = Relocations[s.ImmediateRelocation].TargetOldIndex;

               // Look up in symbol table
               SymNewI = Symbols.Old2NewIndex(SymOldI);
               if (Symbols[SymNewI].OldIndex) {
                  // Found
                  // Check if symbol already has a scope assigned
                  if (Symbols[SymNewI].Scope == 0) Symbols[SymNewI].Scope = 2;

                  // Check if symbol already has a type assigned
                  if ((OperandType & 0xFF) > (Symbols[SymNewI].Type & 0xFF)) {

                     // No type assigned yet, or new type overrides old type
                     Symbols[SymNewI].Type = (Symbols[SymNewI].Type & ~0xFF) | OperandType;
                  }
                  // Check if jump target is in data segment
                  if (Symbols[SymNewI].Section > 0 && (uint16)(Symbols[SymNewI].Section) < Sections.GetNumEntries()
                  && (Sections[Symbols[SymNewI].Section].Type & 0xFF) > 1) {
                     s.Warnings |= 0x80000;
                  }
               }
            }
         }
         else {
            // Check if reference to data symbol
            if ((s.Operands[OpI] & 0x300) && (s.Operands[OpI] & 0x60000)) {
               // Memory operand

               if (s.AddressRelocation) {
                  // There is a reference to a data symbol

                  // Make exception for LEA: Target type is unknown
                  if (Opcodei == 0x8D) OperandType = 0;

                  // Check and update relocation target
                  CheckRelocationTarget(s.AddressRelocation, OperandType, GetDataItemSize(OperandType));
               }
               else if (s.AddressFieldSize >= 4) {
                  // Relocation missing. Make one if possible
                  uint32 TargetType = OperandType;
                  if (Opcodei == 0x8D) {
                     // Source of LEA instruction has no type
                     TargetType = 0;
                  }
                  // Check addressing mode
                  if (s.MFlags & 0x100) {
                     // There is a rip-relative reference
                     // Make relocation record and target record
                     MakeMissingRelocation(Section, s.AddressField, 2, TargetType, 2);
                     FindRelocations();
                  }
                  else if (s.BaseReg && t.Regist[s.BaseReg-1] == 1 && s.AddressFieldSize == 4) {
                     // Memory operand has a base register which has been traced
                     // to contain the image base. Make image-relative relocation
                     MakeMissingRelocation(Section, s.AddressField, 4, TargetType, 2);
                     FindRelocations();
                  }
                  else if (ImageBase && !(RelocationsInSource & 0x20) && s.AddressFieldSize >= 4) {
                     // No base relocations in source. Make direct relocation
                     MakeMissingRelocation(Section, s.AddressField, 1, TargetType, 2, s.AddressFieldSize);
                     FindRelocations();
                  }
               }
            }
            if (OpI == 2 && (s.Operands[OpI] & 0x3000)) {
               // Immediate operand

               if (!s.ImmediateRelocation && s.ImmediateFieldSize >= 4 
               && ImageBase && !(RelocationsInSource & 0x20)
               && (Opcodei == 0x3000 || Opcodei == 0x68 || (Opcodei & 0xFFF8) == 0xB8)) {
                  // instruction = MOV or PUSH, immediate operand may be an address
                  // Make a relocation if immediate value is valid address
                  MakeMissingRelocation(Section, s.ImmediateField, 1, 0, 2, s.ImmediateFieldSize);
                  FindRelocations();
               }
               if (s.ImmediateRelocation) {
                  // There is a reference to the offset of a data symbol
                  // Check and update relocation target
                  CheckRelocationTarget(s.ImmediateRelocation, 0, 0);
               }
            }
         }
         if (((OperandType + 1) & 0xFE) == 0x0C) {
            // Indirect jump or call. Find jump table or virtual table

            // Default relocation type for jump table is direct
            uint32 RelocationType = 1;

            // Find symbol table entry for jump pointer or call pointer
            if (s.AddressRelocation && Relocations[s.AddressRelocation].TargetOldIndex) {
               // Look up in symbol table
               SymNewI = Symbols.Old2NewIndex(Relocations[s.AddressRelocation].TargetOldIndex);
            }
            else SymNewI = 0;

            if (SymNewI == 0 || Symbols[SymNewI].OldIndex == 0) {
               // Symbol for jump table not found yet
               if (s.Operands[OpI] & 0x300) {
                  // There is a memory operand
                  if (s.BaseReg && (t.Regist[s.BaseReg-1] & 0xFE) == 0x18) {
                     // Memory operand has a base register which has been traced to
                     // point to a known symbol
                     SymNewI = Symbols.Old2NewIndex(t.Value[s.BaseReg-1]);
                  }
               }
               else {
                  // Jump or call to a register operand
                  // Check if the register value has been traced
                  if ((t.Regist[s.RM] & 0x1C) == 0x10) {
                     // Register contains an array element. Get symbol for this array
                     SymNewI = Symbols.Old2NewIndex(t.Value[s.RM]);
                     // Check relocation type
                     if (t.Regist[s.RM] == 0x12) {
                        // Register contains array element plus imagebase. 
                        RelocationType = 4;         // Array elements must have image-relative relocations
                     }
                  }
               }
            }
            // Check if valid symbol for jump/call table
            if (SymNewI && Symbols[SymNewI].OldIndex) {
               // Jump/call table found

               if ((s.Operands[OpI] & 0x300) && !s.BaseReg && !s.IndexReg && Opcodei == 0x2704) {
                  // Simple memory operand
                  // Assign name if symbol is import table entry
                  CheckImportSymbol(SymNewI);
               }

               // Check relocation type if memory operand
               if ((s.Operands[OpI] & 0x300) && s.BaseReg && t.Regist[s.BaseReg-1] == 1) {
                  // Memory operand has a base register which has been traced to contain the imagebase
                  RelocationType = 4;               // Array elements must have image-relative relocations
               }

               // Check symbol type
               if ((Symbols[SymNewI].Type & 0xFF) < (OperandType & 0xFF) || (Symbols[SymNewI].Type & 0xF0)) {
                  // No type assigned yet, or new type overrides old type
                  Symbols[SymNewI].Type = OperandType;
               }

               // Check symbol size
               if (RelocationType == 4 && WordSize > 16) {
                  Symbols[SymNewI].Size = 4;     // Image relative
               }
               else {
                  Symbols[SymNewI].Size = WordSize / 8; // Direct
               }

               // Follow what the jump/call table points to
               FollowJumpTable(SymNewI, RelocationType);
            }
         }
      }
   }
}

void CDisassembler::FollowJumpTable(uint32 symi, uint32 RelType) {
   // Check jump/call table and its targets
   uint32 sym1, sym2, sym3 = 0;                  // Symbol indices
   uint32 NextLabel;                             // Offset of next label
   uint32 Pos;                                   // Current position
   SARelocation rel;                             // Relocation record for searching
   int32  Reli;                                  // Index to relocation
   uint32 NewType = 0;                           // Type to assign to symbol
   int32  SourceSection;                         // Section of relocation source
   uint32 SourceOffset;                          // Offset of relocation source
   uint32 SourceSize;                            // Size of relocation source
   uint32 TargetType;                            // Type for relocation target

   // Check if sym is  valid
   if (Symbols[symi].OldIndex == 0) return;

   // Get type of target
   switch (s.OpcodeDef->Destination & 0xFF) {
   case 0x0B:  // Near indirect jump. Target type = jump destination
      NewType = 0x82;  break;
   case 0x0C:  // Near indirect call. Target type = call destination
      NewType = 0x83;  break;
   default:    // Should not occur
      return;
   }

   // Check symbol size
   if ((RelType & 4) && WordSize >= 32) {
      // Image relative relocation
      Symbols[symi].Size = 4;
   }
   else if ((RelType & 0x21) || Symbols[symi].Size == 0) {
      // Direct near relocation
      Symbols[symi].Size = WordSize / 8;
   }

   // Check symbol type
   if (uint32(s.OpcodeDef->Destination & 0xFF) > (Symbols[symi].Type & 0xFF)) {
      // No type assigned yet, or new type overrides old type
      Symbols[symi].Type = s.OpcodeDef->Destination | 0x4000000;
   }
   // Make sure symbol is marked as data
   Symbols[symi].Type |= 0x4000000;   
   
   // Check if symbol has a scope assigned
   if (Symbols[symi].Scope == 0) Symbols[symi].Scope = 2;

   // Save symbol properties
   // (The reference to sym will become invalid when new symbols are created)
   SourceSection = Symbols[symi].Section;
   SourceOffset  = Symbols[symi].Offset;
   SourceSize    = Symbols[symi].Size;
   TargetType    = 0x82;                         // Target type = jump label
   if ((Symbols[symi].Type & 0xFF) == 0x0C) TargetType++;  // Target type = call label

   // Find next label
   sym1 = Symbols.FindByAddress(SourceSection, SourceOffset, &sym2, &sym3);
   if (sym1 && sym3) {
      // Assume that table ends at next label
      NextLabel = Symbols[sym3].Offset;
   }
   else {
      // No next label. End at source section end
      NextLabel = Sections[SourceSection].InitSize;
   }

   // Loop through table of jump/call addresses
   for (Pos = SourceOffset; Pos < NextLabel; Pos += SourceSize) {

      // Search for relocation source at table entry
      rel.Section = SourceSection;
      rel.Offset  = Pos;
      Reli = Relocations.Exists(rel);

      if (Reli > 0) {
         // Relocation found. Check target
         CheckRelocationTarget(Reli, TargetType, 0);
      }
      else {
         // No relocation here. Make one if possible

         uint32 symi = MakeMissingRelocation(rel.Section, rel.Offset, RelType, TargetType, 2);
         if (!symi) {
            // Failed to make a meaningful relocation. End jump table
            break;
         }
         int32 TargetSection = Symbols[symi].Section;
         if (!TargetSection || (Sections[TargetSection].Type & 0xFF) != 1) {
            // Target is not in code section. End jump table
            break;
         }
         // Find the newly made relocation
         Reli = Relocations.Exists(rel);
         if (Reli <= 0) break;
      }
      // Relocation found. Check if valid
      if (!(Relocations[Reli].Type & 0x25) || !Relocations[Reli].TargetOldIndex) {
         // Wrong relocation type or invalid. Stop searching
         break;
      }
      // Find relocation target
      uint32 TargetSymI = Symbols.Old2NewIndex(Relocations[Reli].TargetOldIndex);
      if (!TargetSymI) {
         // Target invalid
         break;
      }
      // Update target symbol type
      if ((Symbols[TargetSymI].Type & 0xFF) < NewType) {
         Symbols[TargetSymI].Type = (Symbols[TargetSymI].Type & ~0xFF) | NewType;
      }
      // Extend current function to include target
      CheckJumpTarget(TargetSymI);
   }

   if (Pos < NextLabel) {
      // There is no label after jump table. Make one with zero scope
      SASymbol SymAfter;
      SymAfter.Reset();
      SymAfter.Section = SourceSection;
      SymAfter.Offset  = Pos;
      SymAfter.Type    = (Sections[SourceSection].Type & 0xFF) == 1 ? 0x82 : 0;
      Symbols.NewSymbol(SymAfter);
   }
}

uint32 CDisassembler::MakeMissingRelocation(int32 Section, uint32 Offset, uint32 RelType, uint32 TargetType, uint32 TargetScope, uint32 SourceSize) {
   // Make a relocation and its target symbol from inline address
   /* This function is used for executable files that have already been 
      relocated for making the relocation information that has been
      lost as well as the symbol record that the relocation should
      point to.
      Parameters:
      Section     Section of relocation source
      Offset      Offset of relocation source
      RelType     Relocation type: 1 = direct, 2 = self relative, 4 = image relative
      TargetType  Symbol type for target
      TargetScope Scope for target symbol
      SourceSize  Size of source field (0 = default for relocation type and WordSize)

      The return value is a symbol new index for the target, or zero if failure

      The size of the relocation source is implied from RelType
      A symbol record for the target will be made if none exists. 
      The scope of the target symbol will be file local (2)
      */

   SARelocation Rel;                             // Temporary relocation record
   SASymbol Sym;                                 // Temporary symbol record for target
   Sym.Reset();
   int32  irel;                                  // Relocation index
   uint32 isym = 0;                              // Symbol new index
   int64  InlineA;                               // Inline address or displacement
   int64  TargetAbsAddr;                         // Absolute address of target

   // Check if Section valid
   if (Section <= 0 || (uint32)Section >= Sections.GetNumEntries() || Offset >= Sections[Section].InitSize || !Sections[Section].Start) {
      return 0;
   }

   // Check if a relocation would be missing
   if (RelType & 1) {
      // Direct relocation
      if (RelocationsInSource & 0x20) return 0;  // Source file has base relocations. There would be a relocation here if needed
   }
   else if (RelType & 4) {
      // Image relative
      if (!ExeType) return 0;                    // Object file. There would be a relocation here if needed
   }

   // Check if a relocation already exists
   Rel.Section = Section;
   Rel.Offset  = Offset;
   irel = Relocations.Exists(Rel);
   if (irel > 0) return 0;                       // Relocation exists. Don't do anything

   if (SourceSize == 0) {
   // Source size not specified. Get default source size
      if ((TargetType & 0xFF) == 0x81) {
         // Short jump
         SourceSize = 1;
      }
      else if (RelType & 1) {
         // Direct relocation. Size depends on word size
         SourceSize = WordSize / 8;
      }
      else if (RelType & 2) {
         // Self relative
         SourceSize = (WordSize == 16) ? 2 : 4;
      }
      else if (RelType & 4 && WordSize > 16) {
         // Image relative
         SourceSize = 4;
      }
      else {
         // Other value. Ignore
         return 0;
      }
   }

   // Get inline address or displacement from source address
   if (SourceSize == 8) {
      InlineA = *(int64*)(Sections[Section].Start + Offset);
   }
   else if (SourceSize == 4) {
      InlineA = *(int32*)(Sections[Section].Start + Offset);
   }
   else if (SourceSize == 2) {
      InlineA = *(int16*)(Sections[Section].Start + Offset);
   }
   else { // 1
      InlineA = *(int8*)(Sections[Section].Start + Offset);
   }

   // Get absolute virtual address of target
   if (RelType & 1) {
      // Direct address
      TargetAbsAddr = InlineA;
   }
   else if (RelType & 2) {
      // Self relative. Translate self-relative to absolute address
      TargetAbsAddr = InlineA + ImageBase + SectionAddress + IEnd;
   }
   else {
      // Image relative
      TargetAbsAddr = InlineA + ImageBase;
   }

   if (ExeType) {
      // Executable file
      // Translate to section:offset address
      if (TranslateAbsAddress(TargetAbsAddr, Sym.Section, Sym.Offset)) {

         // Make a symbol for this address if none exists
         Sym.Scope = TargetScope;
         Sym.Type  = TargetType;
         isym = Symbols.NewSymbol(Sym);
      }
      else if (TargetAbsAddr == ImageBase && TargetAbsAddr) {
         // Reference to image base (nonzero)
         // Make a symbol for image base if none exists
         Sym.Scope = 0x20;
         Sym.Type  = 0;
         isym = Symbols.NewSymbol(Sym);
         if (isym && Symbols[isym].Name == 0) {
            Symbols.AssignName(isym, "__ImageBase");
         }
      }
   }
   else {
      // Object file
      Sym.Section = Section;
      Sym.Offset  = (uint32)TargetAbsAddr - SectionAddress;

      // Make a symbol for this address if none exists
      Sym.Scope = TargetScope;
      Sym.Type  = TargetType;
      isym = Symbols.NewSymbol(Sym);
   }

   if ((RelType & 2) && (TargetType & 0xF0) == 0x80 && Sym.Section == Section && CodeMode == 1) {
      // Relocation not needed for relative jump/call within same section
      return isym;
   }

   if (isym) {
      // Relocation addend
      int32 Addend = -(int32)InlineA;
      if (RelType & 2) {
         // Correct self-relative record for bias
         if (s.MFlags & 0x100) {
            // rip-relative address
            Addend -= IEnd - s.AddressField;
         }
         else {
            // self-relative jump etc.
            Addend -= SourceSize;
         }
      }

      // Make a relocation record
      AddRelocation (Section, Offset, Addend, RelType, SourceSize, Symbols[isym].OldIndex);

      // Update s.AddressRelocation and s.ImmediateRelocation
      if (CodeMode & 3) {
         FindRelocations();

         // Remove warning for absolute address
         s.Warnings &= ~0x8000;
      }
   }
   return isym;
}

void CDisassembler::CheckImportSymbol(uint32 symi) {
   // Check for indirect jump to import table entry

   if (Symbols[symi].DLLName) {
      // Instruction is an indirect jump to symbol table entry
      // Find label at current instruction
      uint32 sym2 = Symbols.FindByAddress(Section, IBegin);
      if (sym2 && Symbols[sym2].Name == 0) {
         // Label at current instruction has no name
         // Give current instruction the import name without "_imp" prefix
         char * ImpName = Symbols.GetName(symi);
         if (strncmp(ImpName, Symbols.ImportTablePrefix, (uint32)strlen(Symbols.ImportTablePrefix)) == 0) {
            Symbols.AssignName(sym2, ImpName + (uint32)strlen(Symbols.ImportTablePrefix));
         }
      }
   }
}

void CDisassembler::MarkCodeAsDubious() {
   // Remember that this may be data in a code segment
   uint32 sym1, sym2 = 0, sym3 = 0;              // Preceding and succeding symbols

   // Check likelihood that this is data rather than code
   if ((s.Errors & 0x4000) && ((s.Warnings & 0x10000000) || CountErrors > 1)
   || CountErrors > 5) {
      // There are more than 5 errors, or consecutive zeroes and at
      // least one more error or inaccessible code.
      // Consider this sufficient evidence that this is very unlikely 
      // to be code. Show it as data only
      CodeMode = 4;
   }
   if (CodeMode < 4) {
      // This may be code containing errors or interpreted out of phase.
      // Set CodeMode to dubious so that it will be shown as both code and data
      CodeMode = 2;
   }

   if (Pass & 0x0F) {
      // Pass 1. Mark preceding label as dubious

      // Check nearest preceding label
      if (LabelBegin == 0) {
         // There is no preceding label. Make one
         Symbols.NewSymbol(Section, IBegin, 1);
         LabelBegin = 0;
         FindLabels();
      }

      // Find symbol index for nearest preceding label
      sym1 = Symbols.FindByAddress(Section, LabelBegin, &sym2, &sym3);

      if (sym1 && sym2) {
         // Mark symbol as dubious or data
         Symbols[sym2].Type = (Symbols[sym2].Type & ~0xF000000) | (CodeMode << 24);
      }

      // Request repetition of pass 1
      Pass |= 0x100;

      /* Skip to next label.
      This is removed because we want to accumulate errors as evidence for 
      determined whether this is code or data
      // Is there a label after this?
      if (sym3) {
         // Skip to next label
         if (Symbols[sym3].Offset > IEnd) {
            IBegin = IEnd = Symbols[sym3].Offset;
         }
      }
      else {
         // No next label. Skip to section end
         IBegin = IEnd = SectionEnd;
      }
      */
   }
}


void CDisassembler::CountInstructions() {
   // Count total number of instructions defined in opcodes.cpp
   // Two instructions are regarded as the same and counted as one if they 
   // have the same name and differ only in the bits that define register 
   // name, operand size, etc.

   uint32 map;                                   // Map number
   uint32 index;                                 // Index into map
   uint32 n;                                     // Number of instructions with same code
   uint32 iset;                                  // Instruction set
   uint32 instructions = 0;                      // Total number of instructions
   uint32 mmxinstr = 0;                          // Number of MMX instructions
   uint32 sseinstr = 0;                          // Number of SSE instructions
   uint32 sse2instr = 0;                         // Number of SSE2 instructions
   uint32 sse3instr = 0;                         // Number of SSE3 instructions
   uint32 sse4instr = 0;                         // Number of SSE4 and later instructions
   uint32 AMDinstr = 0;                          // Number of AMD instructions
   uint32 privilinstr = 0;                       // Number of privileged instructions
   uint32 undocinstr = 0;                        // Number of undocumented instructions
   SOpcodeDef const * opcode;                    // Pointer to map entry

   // Loop through all maps
   for (map = 0; map < NumOpcodeTables1; map++) {
      // Loop through each map
      for (index = 0; index < OpcodeTableLength[map]; index++) {
         opcode = OpcodeTables[map] + index;
         if (opcode->InstructionFormat && opcode->Name 
         && !opcode->TableLink && !(opcode->InstructionFormat & 0x8000)) {
            // instruction is defined
            if ((opcode->InstructionFormat & 0xFFF) == 3
            && index > 0 && (opcode-1)->Name 
            && strcmp(opcode->Name, (opcode-1)->Name) == 0) {
               // Same as previous instruction, just with another register
               continue;                         // Don't count this
            }
            n = 1;                               // Default = one instruction per map entry
            // Check if we have multiple instructions with different prefixes
            if (opcode->Options & 1) {
               if ((opcode->AllowedPrefixes & 0x300) && !(opcode->AllowedPrefixes & 0x8000)) {
                  n++;                           // Extra instruction with 66 prefix
               }
               if (opcode->AllowedPrefixes & 0x400) {
                  n++;                           // Extra instruction with F3 prefix
               }
               if (opcode->AllowedPrefixes & 0x800) {
                  n++;                           // Extra instruction with F2 prefix
               }
            }
            instructions += n;                   // Count instructions
            iset = opcode->InstructionSet;       // Instruction set
            if (iset & 0x800) privilinstr += n;  // Privileged instruction
            if (opcode->InstructionFormat & 0x4000) undocinstr += n; // Undocumented instruction

            switch (iset & 0x17FF) {
            case 7:  // MMX
               mmxinstr += n;  break;
            case 0x11:  // SSE
               sseinstr += n;  break;
            case 0x12:  // SSE2
               sse2instr += n;  break;
            case 0x13: case 0x14: // SSE3 and SSE3B
               sse3instr += n;  break;
            case 0x15: case 0x16: case 0x17: // SSE4 and later
               sse4instr += n;  break;
            case 0x1000: case 0x1001:  // AMD
               AMDinstr += n;  break;
            }
         }
      }
   }

   // output result
   printf("\n\nNumber of instruction opcodes defined:\n%5i Total, including:", 
      instructions);
   printf("\n%5i Privileged instructions", privilinstr);
   printf("\n%5i MMX  instructions", mmxinstr);
   printf("\n%5i SSE  instructions", sseinstr);
   printf("\n%5i SSE2 instructions", sse2instr);
   printf("\n%5i SSE3 instructions", sse3instr);
   printf("\n%5i SSE4 and later instructions", sse4instr);
   printf("\n%5i AMD  instructions", AMDinstr);
   printf("\n%5i Undocumented or illegal instructions", undocinstr);   
}

int CDisassembler::NextInstruction1() {
   // Go to next instruction or data item. Return 0 if none. Pass 1
   IBegin = IEnd;

   // Reset everything in s field
   s.Reset();

   // Return if there are more instructions
   return (IBegin < SectionEnd);
}

int CDisassembler::NextInstruction2() {
   // Go to next instruction or data item. Return 0 if none. Pass 2
   IBegin = IEnd;

   // Reset everything in s field
   s.Reset();

   // Return if there are more instructions
   return (IBegin < FunctionEnd && IBegin < LabelEnd && IBegin < SectionEnd);
}

void CDisassembler::ParseInstruction() {
   // Parse one opcode
   FlagPrevious = 0;                             // Reset flag from previous instruction

   s.OpcodeStart1 = IBegin;                      // Index to start of instruction

   // Scan prefixes first
   ScanPrefixes();

   // Find opcode map entry
   FindMapEntry();                               // Find entry in opcode maps

   // Find operands
   FindOperands();                               // Interpret mod/reg/rm and SIB bytes and find operands

   // Determine the types of each operand
   FindOperandTypes();

   // Find any relocation sources in this instruction
   FindRelocations();

   // Find any reasons for warnings
   FindWarnings();

   // Find any errors
   FindErrors();

   if (!s.Errors && CodeMode == 1) {
      // Find instruction set
      FindInstructionSet();

      // Update symbol types for operands of this instruction
      UpdateSymbols();

      // Trace register values
      UpdateTracer();
   }
}


void CDisassembler::ScanPrefixes() {
   // Scan prefixes
   uint32 i;                                     // Index to current byte
   uint8  Byte;                                  // Current byte of code
   for (i = IBegin; i < SectionEnd; i++) {

      // Read code byte
      Byte = Buffer[i];

      // Check if Byte is a prefix
      if (WordSize == 64 && (Byte & 0xF0) == 0x40) {

         // This is a REX prefix
         if (Byte & 0x08) {
            // REX.W prefix
            StorePrefix(4, 0x48);                // REX.W also in category operand size
         }
         StorePrefix(7, Byte);                   // Store in category REX
      }
      else if (OpcodeMap0[Byte].InstructionFormat & 0x8000) {

         // This is a prefix (other than REX)
         switch (Byte) {
         case 0x26: case 0x2E: case 0x36: case 0x3E: case 0x64: case 0x65:
            // Segment prefix
            StorePrefix(0, Byte);                // Store prefix
            if (Byte == 0x64) MasmOptions |= 2;  // Remember FS used
            if (Byte == 0x65) MasmOptions |= 4;  // Remember GS used
            break;

         case 0x67:
            // Address size prefix
            StorePrefix(1, Byte);  break;

         case 0xF0:
            // Lock prefix
            StorePrefix(2, Byte);  break;

         case 0xF2: case 0xF3:
            // Repeat prefix
            StorePrefix(3, Byte);  // Both in category repeat and operand type
            StorePrefix(5, Byte);  break;

         case 0x66:
            // Operand size
            StorePrefix(4, Byte);  // Both in category operand size and operand type
            StorePrefix(5, Byte);  break;

         default:
            err.submit(9000);
         }
      }
      else {
         // This is not a prefix
         s.OpcodeStart1 = i;
         return;
      }
   }
   // Error: end of block reached before end of prefixes
   IEnd = i;
   s.Errors |= 0x10;
}

void CDisassembler::StorePrefix(uint32 Category, uint8 Byte) {
   // Store prefix according to category
   if (Category > 7) {err.submit(9000); return;} // Out of range

   // Check if we already have a prefix in this category
   if (s.Prefixes[Category]) {
      // We already have a prefix in this category
      if (s.Prefixes[Category] != Byte || Category == 7) {
         // Conflicting prefixes in this category
         s.Conflicts[Category]++;
      }
      else {
         // Same prefix occurs more than once
         s.Warnings |= 0x100;
      }
   }
   // Check if REX prefix before this
   if (s.Prefixes[7]) s.Errors |= 0x20;

   // Save prefix in category
   s.Prefixes[Category] = Byte;
}


void CDisassembler::FindMapEntry() {
   // Find entry in opcode maps
   uint32 i = s.OpcodeStart1;                    // Index to current byte
   uint8  Link;                                  // Link to another map
   uint8  Byte = Buffer[i];                      // Current byte of code or index into map
   uint32 MapNumber = 0;                         // Map number in opcodes.cpp
   SOpcodeDef const * MapEntry;                  // Point to current opcode map entry
   MapEntry = OpcodeMap0 + Byte;                 // Get entry [Byte] in first map

   // Save previous opcode and options
   *(uint32*)&PreviousOpcodei = *(uint32*)&Opcodei;
   *(uint32*)&Opcodei = 0;

   // Loop through map tree (exit loop when Link == 0)
   while (1) {

      // Check if MapEntry has a link to another map
      Link = MapEntry->TableLink;

      switch (Link) {
      case 0:      // No link
         // Final map entry found
         s.OpcodeStart2 = i;
         s.OpcodeDef    = MapEntry;

         // Save opcode and options
         Opcodei = (MapNumber << 8) | Byte;
         OpcodeOptions = MapEntry->Options;

         // Return success
         return;

      case 1:      // Use following byte as index into next table
         if (i >= SectionEnd) {
            // Instruction extends beyond end of block
            IEnd = i;  s.Errors |= 0x10;
            s.OpcodeStart2 = i;
            return;
         }
         Byte = Buffer[++i];                     // Get next byte of code as index
         break;

      case 2:      // Use reg field of mod/reg/rm byte as index into next table
         Byte = (Buffer[i+1] >> 3) & 7;          // Read reg bits
         break;

      case 3:      // Use mod < 3 vs. mod == 3 as index into next table
         Byte = (Buffer[i+1] & 0xC0) == 0xC0;    // 1 if mod == 3
         break;

      case 4:      // Use mod and reg fields of mod/reg/rm byte as index into next table,
         // first 8 entries indexed by reg for mod < 3, next 8 entries indexed by reg for mod = 3.
         Byte = (Buffer[i+1] >> 3) & 7;          // Read reg bits
         if ((Buffer[i+1] & 0xC0) == 0xC0) Byte += 8; // Add 8 if mod == 3
         break;

      case 5:      // Use rm bits of mod/reg/rm byte as index into next table
         Byte = Buffer[i+1] & 7;                 // Read r/m bits
         break;

      case 6:      // Use immediate byte after any other operands as index into next table
         s.OpcodeStart2 = i;
         s.OpcodeDef    = MapEntry;
         FindOperands();                         // Find size of all operand fields and end of instruction
         Byte = Buffer[IEnd - 1];                // Last byte of instruction
         break;

      case 7:      // Use mode as index into next table (16, 32, 64 bits)
         switch (WordSize) {
         case 16:
            Byte = 0;  break;
         case 32: default:
            Byte = 1;  break;
         case 64: 
            Byte = 2;
         }
         break;

      case 8:      // Use operand size as index into next table (16, 32, 64 bits)
         switch (WordSize) {
         case 64:
            if (s.Prefixes[4] == 0x48) {         // REX.W prefix = 64 bit
               Byte = 2;  break;
            }
            // Else continue in case 32:
         case 32: default:
            Byte = (s.Prefixes[4] == 0x66) ? 0 : 1;  break;
         case 16:
            Byte = (s.Prefixes[4] == 0x66) ? 1 : 0;  break;
         }
         break;

      case 9:      // Use operand type prefixes as index into next table (none, 66, F2, F3)
         switch (s.Prefixes[5]) {
         case 0: default:
            Byte = 0;  break;
         case 0x66:
            Byte = 1;  break;
         case 0xF2:
            Byte = 2;  break;
         case 0xF3:
            Byte = 3;  break;
         }
         break;

      case 0xA:    // Use address size as index into next table (16, 32, 64 bits)
         switch (WordSize) {
         case 64:
            Byte = (s.Prefixes[1] == 0x67) ? 1 : 2;  break;
         case 32: default:
            Byte = (s.Prefixes[1] == 0x67) ? 0 : 1;  break;
         case 16:
            Byte = (s.Prefixes[1] == 0x67) ? 1 : 0;  break;
         }
         break;

      case 0x10:   // Use assembly language dialect as index into next table
         // Not supported yet

      default:     // Internal error in map tree
         err.submit(9007, MapNumber);
         s.OpcodeStart2 = i;
         return;
      }

      // Get next map from branched tree of maps
      MapNumber = MapEntry->InstructionSet;
      if (MapNumber >= NumOpcodeTables1 || OpcodeTableLength[MapNumber] == 0) {
         err.submit(9007, MapNumber);  return;   // Map number out of range
      }

      // Use Byte as index into new map. Check if within range
      if (Byte >= OpcodeTableLength[MapNumber]) {
         // Points outside map. Get last entry in map containing default
         Byte = OpcodeTableLength[MapNumber] - 1;
      }
      // Point to entry [Byte] in new map
      MapEntry = OpcodeTables[MapNumber] + Byte;
      if (MapEntry == 0) {
         err.submit(9007, MapNumber);  return;   // Map missing
      }

   }  // Loop end. Go to next
}


void CDisassembler::FindOperands() {
   // Interpret mod/reg/rm and SIB bytes and find operands
   s.MFlags = 0;                                 // Memory operand flags:
                                                 // 1 = has memory operand, 
                                                 // 2 = has mod/reg/rm byte, 
                                                 // 4 = has SIB byte, 
                                                 // 0x10 = is rip-relative
   uint8 ModRegRM;                               // mod/reg/rm byte 

   // Get address size   
   if (WordSize == 64) s.AddressSize = (s.Prefixes[1] == 0x67) ? 32 : 64;
   else s.AddressSize = (WordSize == 16) ^ (s.Prefixes[1] == 0x67) ? 16 : 32;

   s.AddressFieldSize = s.ImmediateFieldSize = 0;// Initialize

   // Check if there is a mod/reg/rm byte
   if (s.OpcodeDef->InstructionFormat & 0x10) {

      // There is a mod/reg/rm byte
      s.MFlags |= 2;

      if (s.OpcodeStart2 + 1 >= FunctionEnd) {
         // extends outside code block
         s.Errors |= 0x10;  
      }
      
      // Read mod/reg/rm byte
      ModRegRM = Buffer[s.OpcodeStart2 + 1];
      s.Mod =  ModRegRM >> 6;                    // mod = bit 6-7
      s.Reg = (ModRegRM >> 3) & 7;               // reg = bit 3-5
      s.RM  =  ModRegRM & 7;                     // RM  = bit 0-2

      // Check REX prefix
      if (s.Prefixes[7] & 4) s.Reg |= 8;         // Add REX.R to reg field
      if (s.Prefixes[7] & 1) s.RM  |= 8;         // Add REX.B to RM  field

      // Position of address/displacement field, if any
      // (Will be modified later if SIB byte found)
      s.AddressField = s.OpcodeStart2 + 2;       // Beginning of address/displacement field

      // Interpretation of mod/reg/rm byte is different for 16 bit address size
      if (s.AddressSize == 16) {

         if (s.Mod != 3) {
            // There is a memory operand
            s.MFlags |= 1;

            // Get size of address/displacement operand from mod bits
            // (Will be overwritten later if none)
            if (s.Mod == 1) {
               s.AddressFieldSize = 1;           // Size of displacement field
            }
            else if (s.Mod == 2) {
               s.AddressFieldSize = 2;           // Size of displacement field
            }

            // Check if direct memory operand
            if (s.Mod == 0 && s.RM == 6) {
               // Direct memory operand and nothing else
               s.AddressFieldSize = 2;           // Size of address field
            }
            else {
               // Indirect memory operand
               // Get base and index registers
               // [bx+si], [bx+di], [bp+si], [bp+di], [si], [di], [bp], [bx]
               static const uint8 BaseRegister [8] = {3+1, 3+1, 5+1, 5+1, 0, 0, 5+1, 3+1};
               static const uint8 IndexRegister[8] = {6+1, 7+1, 6+1, 7+1, 6+1, 7+1, 0, 0};
               // Save register number + 1, because 0 means none.
               s.BaseReg  = BaseRegister [s.RM]; // Base register = BX or BP or none
               s.IndexReg = IndexRegister[s.RM]; // Index register = SI or DI or none
               s.Scale = 0;                      // No scale factor in 16 bit mode
            }
         }
      }
      else {
         // Address size is 32 or 64 bits

         if (s.Mod != 3) {
            // There is a memory operand
            s.MFlags |= 1;

            // Get size of address/displacement operand from mod bits
            // (Will be overwritten later if none)
            if (s.Mod == 1) {
               s.AddressFieldSize = 1;              // Size of displacement field
            }
            else if (s.Mod == 2) {
               s.AddressFieldSize = 4;              // Size of displacement field
            }

            // Check if direct memory operand
            if (s.Mod == 0 && (s.RM & 7) == 5) {
               // Direct memory operand and nothing else
               s.AddressFieldSize = 4;           // Size of address field
            }
            else if ((s.RM & 7) == 4) {
               // There is a SIB byte
               s.MFlags |= 4;                    // Remember we have a SIB byte

               if (s.OpcodeStart2 + 2 >= FunctionEnd) {
                  // extends outside code block
                  s.Errors |= 0x10;
               }

               // Read SIB byte
               uint8 SIB = Buffer[s.OpcodeStart2 + 2];
               s.AddressField++;                 // Address field moved one byte

               // Get scale, index, base
               s.Scale = SIB >> 6;               // Scale = bit 6-7
               s.IndexReg = (SIB >> 3) & 7;      // Index = bit 3-5
               s.BaseReg = SIB & 7;              // Base  = bit 0-2

               // Check REX prefix
               if (s.Prefixes[7] & 2) s.IndexReg |= 8; // Add REX.X to index
               if (s.Prefixes[7] & 1) s.BaseReg  |= 8; // Add REX.B to base
               s.RM &= 7;                              // Remove REX.B from RM

               s.BaseReg++;                      // Add 1 so that 0 means none
               if (s.IndexReg == 4) {
                  // No index register
                  s.IndexReg = 0;
               }
               else {
                  s.IndexReg++;                  // Add 1 so that 0 means none
               }

               if (s.Mod == 0 && s.BaseReg == 5+1) {
                  // No base register, 32 bit address
                  s.AddressFieldSize = 4;
                  s.BaseReg = 0;
               }
            }
            else {
               // Indirect memory operand and no SIB byte
               s.BaseReg = s.RM + 1;             // Get base register from RM bits. Add 1 because 0 means none
            }
         }
         // Check if rip-relative
         if (WordSize == 64 && (s.MFlags & 7) == 3 && !s.BaseReg && s.AddressFieldSize == 4) {
            // Memory operand is rip-relative
            s.MFlags |= 0x100;
         }
      }
   }

   // Get operand size
   uint32 OpSizePrefix = 0;
   if (s.Prefixes[4] == 0x66 && (s.OpcodeDef->AllowedPrefixes & 0x100))  OpSizePrefix = 1; // Operand size prefix
   if (s.Prefixes[4] == 0x48 && (s.OpcodeDef->AllowedPrefixes & 0x1000)) OpSizePrefix = 2; // Rex.W prefix
   s.OperandSize = (WordSize == 16) ^ (OpSizePrefix & 1) ? 16 : 32;
   if (OpSizePrefix == 2) s.OperandSize = 64;
   if ((s.OpcodeDef->AllowedPrefixes & 0x2000) && WordSize == 64 && OpSizePrefix != 1) s.OperandSize = 64;

   // Get any immediate operand
   // Offset to immediate operand field, if any
   s.ImmediateField = s.OpcodeStart2 + 1 + (s.MFlags&2)/2 + (s.MFlags&4)/4 + s.AddressFieldSize;

   // Check InstructionFormat for immediate and direct operands
   switch (s.OpcodeDef->InstructionFormat & 0x0FE0) {
   case 0x20:  // Has 2 bytes immediate operand
      s.ImmediateFieldSize = 2;  break;

   case 0x40:  // Has 1 byte immediate operand or short jump
      s.ImmediateFieldSize = 1;  break;

   case 0x60:  // Has 3 bytes immediate operand (enter)
      s.ImmediateFieldSize = 3;  break;

   case 0x80:  // Has 2 or 4 bytes immediate operand or near jump/call
      if ((s.OpcodeDef->Destination & 0xFE) == 0x82) {
         // Near jump/call address size depends on WordSize and operand size prefix, 
         // but not on address size prefix
         s.ImmediateFieldSize = (WordSize == 16) ^ (s.Prefixes[4] == 0x66) ? 2 : 4;
         if (WordSize == 64) s.ImmediateFieldSize = 4; // 66 prefix ignored in 64 bit mode
      }
      else {
         // Size of other immediate data depend on operand size
         s.ImmediateFieldSize = (s.OperandSize == 16) ? 2 : 4;
      }
      break;

   case 0x100:  // Has 2, 4 or 8 bytes immediate operand
      s.ImmediateFieldSize = s.OperandSize / 8;
      break;

   case 0x200:  // Has 2+2 or 4+2 bytes far direct jump/call operand
      s.ImmediateFieldSize = (WordSize == 16) ^ (s.Prefixes[4] == 0x66) ? 4 : 6;
      break;

   case 0x400:  // Has 2, 4 or 8 bytes direct memory operand
      s.AddressFieldSize = s.AddressSize / 8;
      s.AddressField = s.ImmediateField;
      s.ImmediateField = s.AddressField + s.AddressFieldSize;
      s.ImmediateFieldSize = 0;
      break;

   default:     // No immediate operand
      s.ImmediateFieldSize = 0;
   }

   // Find instruction end
   IEnd = s.ImmediateField + s.ImmediateFieldSize;

   if (IEnd > FunctionEnd) {
      // instruction extends outside code block
      s.Errors |= 0x10; 
      IEnd = FunctionEnd;
   }
}

void CDisassembler::FindOperandTypes() {
   // Determine the type of each operand
   uint32 i;                                     // Operands index

   s.Operands[0] = s.OpcodeDef->Destination & 0x7FFF;// Destination operand type
   s.Operands[1] = s.OpcodeDef->Source & 0x07FF; // Source operand type
   s.Operands[2] = s.OpcodeDef->Source & 0xF800; // Immediate source operand or third operand

   // Check instruction format
   switch (s.OpcodeDef->InstructionFormat & 0x13) {

   case 3: // Register operand indicated by bits 0-2 of opcode
      // Find which of the operands it applies to
      if ((s.Operands[0] & 0xFF) > 0 && (s.Operands[0] & 0xFF) < 0xB) i = 0; else i = 1;
      // Indicate this operand uses opcode bits
      s.Operands[i] |= 0x20000; 
      break;

   case 0x11: // There is a mod/reg/rm byte and one operand
      // Find which of the operands it applies to
      if (s.Operands[0] && (s.Operands[0] & 0xFF) < 0xA0) i = 0; else i = 1;
      // Check if the other operand is unused
      if ((s.Operands[1-i] & 0xFF) && (s.Operands[1-i] & 0xFF) < 0xA0) 
         s.Errors |= 0x80000;  // Error in opcode table
      // Indicate this operand uses mod and rm bits
      s.Operands[i] |= 0x20000;
      break;

   case 0x12: // There is a mod/reg/rm byte and two operands. Destination is reg
      // Destination operand uses s.Reg bits
      s.Operands[0] |= 0x10000; 
      // Source operand uses mod and rm bits
      s.Operands[1] |= 0x20000;
      break;

   case 0x13: // There is a mod/reg/rm byte and two operands. Source is reg
      // Source operand uses s.Reg bits
      s.Operands[1] |= 0x10000; 
      // Destination operand uses mod and rm bits
      s.Operands[0] |= 0x20000;
      break;

   default: // No explicit operands. 
      // Check for implicit memory operands
      for (i = 0; i < 2; i++) {
         if (s.Operands[i] & 0x100) {
            // Direct memory operand
            s.Operands[i] |= 0x40000;
            // There is an address field
            s.AddressFieldSize = s.AddressSize / 8; 
            s.AddressField = s.OpcodeStart2 + 1;
            s.MFlags |= 1;                    // Remember we have a memory operand
         }
      }
      break;
   }

   // Loop for destination and source operand
   for (i = 0; i < 2; i++) {

      // Check if register or memory
      switch (s.Operands[i] & 0x300) {
      case 0:  // Must be register
         if ((s.Operands[i] & 0x20000) && s.Mod != 3 && (s.OpcodeDef->InstructionFormat & 0x10)) {
            s.Errors |= 8;                       // Is memory. Indicate wrong operand type
            s.Operands[i] |= 0x100;              // Indicate it is memory
         }
         break;

      case 0x100: // Must be memory operand
         if (!(s.Operands[i] & 0x60000) || s.Mod == 3) {
            s.Errors |= 8;                       // Is register. Indicate wrong operand type
            s.Operands[i] &= ~0x100;             // Indicate it is register
         }
         break;

      case 0x200: case 0x300: // Can be register or memory
         s.Operands[i] &= ~0x300;
         if ((s.Operands[i] & 0x20000) && s.Mod != 3) {
            s.Operands[i] |= 0x100;              // Indicate it is memory
         }
         break;
      }

      // Resolve types that depend on prefixes or WordSize
      switch (s.Operands[i] & 0xFF) {

      case 8:  // 16 or 32 bits
         s.Operands[i] &= ~0xFF;
         s.Operands[i] |= (s.OperandSize == 16) ? 2 : 3;
         break;

      case 9:  // 16, 32 or 64 bits, depending on operand size
         s.Operands[i] &= ~0xFF;
         s.Operands[i] |= (s.OperandSize == 16) ? 2 : ((s.OperandSize == 64) ? 4 : 3);
         break;

      case 0xB: case 0xC: // 16, 32 or 64 bits. Fixed size = 64 in 64 bit mode
         s.Operands[i] &= ~0xFF;
         if (WordSize == 64) {
            s.Operands[i] |= 4;
         }
         else {
            s.Operands[i] |= (s.OperandSize == 16) ? 2 : 3;
         }
         break;

      case 0xA: // 16, 32 or 64 bits. Default size = 64 in 64 bit mode
         s.Operands[i] &= ~0xFF;
         if (WordSize == 64) {
            s.Operands[i] |= (s.OperandSize == 16) ? 2 : 4;
         }
         else {
            s.Operands[i] |= (s.OperandSize == 16) ? 2 : 3;
         }
         break;

      case 0xD: // 16+16, 32+16 or 64+16 bits far indirect pointer (jump or call)
         s.Operands[i] &= ~0xFF;
         s.Operands[i] |= (s.OperandSize == 16) ? 3 : ((s.OperandSize == 64) ? 5 : 7);
         break;

      case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: // mmx or xmm, depending on 66 prefix
         s.Operands[i] &= ~0x30;
         s.Operands[i] |= (s.Prefixes[5] == 0x66) ? 0x20 : 0x10;
         break;

      case 0x6F: // XMM float. Size depends on prefix: none = ps, 66 = pd, F2 = sd, F3 = ss
         s.Operands[i] &= ~0xFF;
         switch (s.Prefixes[5]) {
         case 0:  // No prefix = ps
            s.Operands[i] |= 0x6B;  break;
         case 0x66: // 66 prefix = pd
            s.Operands[i] |= 0x6C;  break;
         case 0xF3: // F3 prefix = ss
            s.Operands[i] |= 0x4B;  break;
         case 0xF2: // F2 prefix = sd
            s.Operands[i] |= 0x4C;  break;
         };
         break;
      }
   }

   // Resolve size of immediate operand
   if (s.Operands[2]) {
      switch (s.ImmediateFieldSize) {
      case 1:  // 1 byte
         s.Operands[2] |= 1;
         break;
      case 2:  // 2 bytes
         s.Operands[2] |= 2;
         break;
      case 3:  // 3 bytes
         s.Operands[2] |= 6;
         break;
      case 4:  // 4 bytes
         s.Operands[2] |= 3;
         break;
      case 6:  // 6 bytes
         s.Operands[2] |= 7;
         break;
      case 8:  // 8 bytes
         s.Operands[2] |= 4;
         break;
      }
   }
}

void CDisassembler::FindWarnings() {
   // Find any reasons for warnings in code
   uint32 i;                                     // Operand index

   if ((s.OpcodeDef->Options & 0x80) && s.ImmediateFieldSize > 1 && s.ImmediateRelocation == 0) {
      // Check if sign-extended operand can be used
      if ((s.ImmediateFieldSize == 2 && Get<int16>(s.ImmediateField) == Get<int8>(s.ImmediateField)) 
      ||  (s.ImmediateFieldSize == 4 && Get<int32>(s.ImmediateField) == Get<int8>(s.ImmediateField))) {
         s.Warnings |= 1; // Sign-extended operand could be used
      }
   }
   if (WordSize == 64 && s.ImmediateFieldSize == 8 && s.ImmediateRelocation == 0) {
      // We have a 64 bit immediate operand. Could it be made shorter?
      if (Get<uint32>(s.ImmediateField+4) == 0) {
         s.Warnings |= 2;                        // Upper half is zero. Could use zero-extension
      }
      else if (Get<int64>(s.ImmediateField) == Get<int32>(s.ImmediateField)) {
         s.Warnings |= 1;                        // Could use sign-extension
      }
   }
   // Check if displacement could be made smaller
   if (s.AddressFieldSize > 0 && s.AddressRelocation == 0 
   && (s.BaseReg || (s.IndexReg && !s.BaseReg && s.Scale < 2))) {
      // There is a displacement which might be unnecessary
      switch (s.AddressFieldSize) {
      case 1:  // 1 byte displacement
         if (Get<uint8>(s.AddressField) == 0 
         && ((s.BaseReg-1 & 7) != 5 || (s.AddressSize == 16 && s.IndexReg))) 
            s.Warnings |= 4; // Displacement is 0 and an addressing mode without displacement exists
         break;
      case 2:  // 2 bytes displacement
         if (Get<int16>(s.AddressField) == 0) s.Warnings |= 4; // Displacement is 0
         else if (Get<int16>(s.AddressField) == Get<int8>(s.AddressField)) s.Warnings |= 8; // Could use sign extension
         break;
      case 4:  // 4 bytes displacement
         if (Get<int32>(s.AddressField) == 0) s.Warnings |= 4; // Displacement is 0
         else if (Get<int32>(s.AddressField) == Get<int8>(s.AddressField)) s.Warnings |= 8; // Could use sign extension
         break;
      case 8:  // 8 bytes displacement
         if (Get<int32>(s.AddressField) == Get<int64>(s.AddressField)) 
            // Has 8 bytes displacement. Could use sign-extended or rip-relative
            s.Warnings |= 8;
         break;
      }
   }
   // Check for unnecessary SIB byte
   if ((s.MFlags&4) && (s.BaseReg&7)!=4+1 && (s.IndexReg==0 || (s.BaseReg==0 && s.Scale==0))) {
      if (WordSize == 64 && s.BaseReg==0 && s.IndexReg==0) s.Warnings |= 0x4000; // 64-bit address not rip-relative
      else s.Warnings |= 0x10; // Unnecessary SIB byte
   }
   // Check if shorter instruction exists for register operands
   if ((s.OpcodeDef->Options & 0x80) && !(s.OpcodeDef->InstructionFormat & 0xFE0) && s.Mod == 3
   && !(WordSize == 64 && Get<uint8>(s.OpcodeStart1) == 0xFF)) {
      s.Warnings |= 0x20;   // No memory operand. A shorter version exists for register operand
   }
   // Check for length-changing prefix
   if (s.ImmediateFieldSize > 1 && s.Prefixes[4] == 0x66 
   && (s.OpcodeDef->AllowedPrefixes & 0x100) && !(s.OpcodeDef->InstructionFormat & 0x20)) {
      // 66 prefix changes length of immediate field
      s.Warnings |= 0x40;
   }
   // Check for bogus length-changing prefix causing stall on Intel Core2.
   // Will occur if 66 prefix and first opcode byte is F7 and there is a 16 bytes boundary between opcode byte and mod/reg/rm byte
   if (Get<uint8>(s.OpcodeStart1) == 0xF7 && s.Prefixes[4] == 0x66 && ((s.OpcodeStart1+1) & 0xF) == 0 && !s.ImmediateFieldSize) {
      s.Warnings |= 0x2000000;
   }
   // Warn for address size prefix if mod/reg/rm byte
   // (This does not cause a stall in 64 bit mode, but I am issueing a 
   // warning anyway because the changed address size is probably unintended)
   if (s.Prefixes[1] == 0x67 && (s.MFlags & 2)) {
      s.Warnings |= 0x80;
   }
   // Check for unnecessary REX.W prefix
   if ((s.OpcodeDef->AllowedPrefixes & 0x2000) && s.Prefixes[7] == 0x48) {
      s.Warnings |= 0x200;  // REX.W prefix valid but unnecessary
   }
   // Check for meaningless prefixes
   if (!(s.OpcodeDef->InstructionFormat & 0x10) || s.Mod == 3) 
      // No mod/reg/rm byte or only register operand. Check for address size and segment prefixes
      if ((s.Prefixes[0] && !(s.OpcodeDef->AllowedPrefixes & 0xC))
      || (s.Prefixes[1] && !(s.OpcodeDef->AllowedPrefixes & 3)))
         s.Warnings |= 0x400; // Unnecessary segment or address size prefix

   // Check for meaningless segment prefixes
   if (s.Prefixes[0] && !(s.OpcodeDef->AllowedPrefixes & 8)) {
      // Segment prefix is not branch hint
      if (WordSize == 64 && (s.Prefixes[0] & 0x02)) 
         s.Warnings |= 0x400; // CS, DS, ES or SS prefix in 64 bit mode has no effect
      if (s.Prefixes[0] == 0x3E && s.BaseReg != 4+1 && s.BaseReg != 5+1) 
         s.Warnings |= 0x400; // Unnecessary DS: segment prefix
      if (s.Prefixes[0] == 0x36 && (s.BaseReg == 4+1 || s.BaseReg == 5+1) )
         s.Warnings |= 0x400; // Unnecessary SS: segment prefix
      if (Opcodei == 0x8D)
         s.Warnings |= 0x400; // Segment prefix on LEA instruction
      if (s.Mod == 3)
         s.Warnings |= 0x400; // mod/reg/rm byte indicates no memory operand
   }

   // Check for meaningless 66 prefix
   if (s.Prefixes[4] == 0x66 && !(s.OpcodeDef->AllowedPrefixes & 0x380))
      s.Warnings |= 0x400; // 66 prefix not allowed here

   // Check for meaningless F2 prefix
   if (s.Prefixes[3] == 0xF2 && !(s.OpcodeDef->AllowedPrefixes & 0x860))
      s.Warnings |= 0x400; // F2 prefix not allowed here

   // Check for meaningless F3 prefix
   if (s.Prefixes[3] == 0xF3 && !(s.OpcodeDef->AllowedPrefixes & 0x460))
      s.Warnings |= 0x400; // F3 prefix not allowed here

   // Check for meaningless REX prefix bits
   if (s.Prefixes[7]) {
      // REX prefix present
      // Get significant bits
      uint8 RexBits = s.Prefixes[7] & 0x0F;
      // Check if empty REX prefix
      if (RexBits == 0 && (s.Operands[0] & 0xFF) != 1 && (s.Operands[1] & 0xFF) != 1) {
         // Empty REX prefix needed only if 8 bit register      
         s.Warnings |= 0x400;
      }
      // Clear bits that are used:
      // Check if REX.W bit used
      if (s.OpcodeDef->AllowedPrefixes & 0x3000) RexBits &= ~8;
      // Check if REX.R and REX.B bit used for source or destination operand
      for (i = 0; i < 2; i++) {
         if ((s.Operands[i] & 0x10000) && (s.Operands[i] & 0xF0) != 0x10
         && (s.Operands[i] & 0x48) != 0x40 && (s.Operands[i] & 0xFF) != 0x91)
            // REX.R used for operand and register type allows value > 7
            RexBits &= ~4;
         if ((s.Operands[i] & 0x20000) && (s.Operands[i] & 0xF0) != 0x10 
         && (s.Operands[i] & 0x48) != 0x40 && (s.Operands[i] & 0xFF) != 0x91)
            // REX.B used for operand and register type allows value > 7
            RexBits &= ~1;
      }
      // Check if REX.X bit used for index register
      if (s.IndexReg) RexBits &= ~2;
      // Check if REX.B bit used for base register
      if (s.BaseReg)  RexBits &= ~1;
      // Check if REX.B bit used for register operand indicated by opcode bits
      if ((s.OpcodeDef->InstructionFormat & 0x1F) == 3) RexBits &= ~1;
      // Any unused bits left?
      if (RexBits) s.Warnings |= 0x400; // At least one REX big makes no sense here
   }

   // Check for conflicting prefixes
   if (s.OpcodeDef->AllowedPrefixes & 0x140)  s.Conflicts[5] = 0; // 66 + F2/F3 allowed for string instructions
   if ((s.OpcodeDef->AllowedPrefixes & 0x1200) == 0x1200) s.Conflicts[4] = 0; // 66 + REX.W allowed for e.g. movd/movq instruction
   if (*(int64*)&s.Conflicts) s.Warnings |= 0x800;  // Conflicting prefixes. Check all categories at once

   // Check for missing prefixes
   if ((s.OpcodeDef->AllowedPrefixes & 0x8000) && s.Prefixes[5] == 0) s.Warnings |= 0x1000; // Required prefix missing

   // Check for unused SIB scale factor
   if (s.Scale && s.IndexReg == 0) s.Warnings |= 0x2000; // SIB has scale factor but no index register

   // Check if address in 64 bit mode is rip-relative
   if (WordSize == 64 && s.AddressFieldSize >= 4 && s.AddressRelocation && !(s.MFlags & 0x100)) {
      // 32-bit address in 64 bit mode is not rip-relative. Check if image-relative
      if (s.AddressRelocation >= Relocations.GetNumEntries() || !(Relocations[s.AddressRelocation].Type & 4)) {
         // Not image-relative either
         s.Warnings |= 0x4000;
      }
   }
   // Check if direct address is relocated
   if (s.AddressFieldSize > 1 && !s.AddressRelocation && !s.BaseReg && !s.IndexReg && (WordSize != 16 || !(s.Prefixes[0] & 0x40))) 
      s.Warnings |= 0x8000;  // Direct address has no relocation, except FS: and GS:

   // Check if address relocation type is correct
   if (s.AddressFieldSize > 1 && s.AddressRelocation && (s.MFlags & 1)) {
      // Memory operand found. Should it be direct or self-relative
      if (s.MFlags & 0x100) {
         // Memory address should be self-relative (rip-relative)
         if (!(Relocations[s.AddressRelocation].Type & 2)) {
            s.Warnings |= 0x10000;     // rip-relative relocation expected but not found
         }
      }
      else {
         // Memory address should be direct
         if (Relocations[s.AddressRelocation].Type & 0x302) {
            s.Warnings |= 0x10000;     // direct address expected, other type found
         }
      }

      // Check if memory address has correct alignment
      // Loop through destination and source operand
      for (i = 0; i < 2; i++) {
         // Operand type
         uint32 OperandType = s.Operands[i];
         if ((OperandType & 0x100) && Opcodei != 0x8D) {
            // This is a memory operand (except LEA). Get target offset
            int64 TargetOffset = 0;
            switch (s.AddressFieldSize) {
            case 1:
               TargetOffset = Get<int8>(s.AddressField);  break;
            case 2:
               TargetOffset = Get<int16>(s.AddressField);  break;
            case 4:
               TargetOffset = Get<int32>(s.AddressField); 
               if (s.MFlags & 0x100) {
                  // Compute rip-relative address
                  TargetOffset += IEnd - s.AddressField;
               }
               break;
            case 8:
               TargetOffset = Get<int64>(s.AddressField);  break;
            }
            // Add relocation offset
            TargetOffset += Relocations[s.AddressRelocation].Addend;

            // Find relocation target
            uint32 SymbolOldIndex = Relocations[s.AddressRelocation].TargetOldIndex;
            uint32 SymbolNewIndex = Symbols.Old2NewIndex(SymbolOldIndex);
            if (SymbolNewIndex) {
               // Add relocation target offset
               TargetOffset += Symbols[SymbolNewIndex].Offset;
               // Target section
               int32 TargetSection = Symbols[SymbolNewIndex].Section;
               if (TargetSection && (uint32)TargetSection < Sections.GetNumEntries()) {
                  // Add relocation section address
                  TargetOffset += Sections[TargetSection].SectionAddress;
               }
               if ((Relocations[s.AddressRelocation].Type & 0x10) && Relocations[s.AddressRelocation].RefOldIndex) {
                  // Add offset of reference point
                  uint32 RefIndex = Symbols.Old2NewIndex(Relocations[s.AddressRelocation].RefOldIndex);
                  TargetOffset += Symbols[RefIndex].Offset;
               }
               if (Relocations[s.AddressRelocation].Type & 0x3000) {
                  // GOT entry etc. Can't check alignment
                  continue;
               }
            }

            // Check operand size
            switch (GetDataItemSize(OperandType)) {
            default:
            case 1:   // 8 bits. No alignment
               break;
            case 2:   // 16 bits. Alignment by 2 recommended
               if (TargetOffset & 1) s.Warnings |= 0x400000; // Warn if not aligned by 2
               break;
            case 4:   // 32 bits. Alignment by 4 recommended
            case 6:   // 48 bits. Alignment by 4 recommended
            case 10:  // 80 bits. Alignment by 4 or 8 recommended
               if (TargetOffset & 3) s.Warnings |= 0x400000; // Warn if not aligned by 4
               break;
            case 8:   // 64 bits. Alignment by 8 recommended
               if (TargetOffset & 7) s.Warnings |= 0x400000; // Warn if not aligned by 8
               break;
            case 16:  // 128 bits. Alignment by 16 required
               if (TargetOffset & 0xF) s.Warnings |= 0x800000; // Error if not aligned by 16
               break;
            }
         }
      }
   }

   // Check if jump relocation type is correct
   if (s.ImmediateFieldSize > 1 && s.ImmediateRelocation && (s.OpcodeDef->Destination & 0xFE) == 0x82) 
      // Near jump or call. Relocation must be self-relative
      if (!(Relocations[s.ImmediateRelocation].Type & 2)) {
         s.Warnings |= 0x10000;  // Self-relative relocation expected but not found
      }
   // Check operand size for jumps
   if ((s.OpcodeDef->AllowedPrefixes & 0x80) && s.Prefixes[4]) {
      // Jump instruction sensitive to operand size prefix
      if (WordSize == 32) s.Warnings |= 0x20000; // Instruction pointer truncated
      if (WordSize == 64) s.Warnings |= 0x400;   // Prefix has no effect
   }

   // Check address size for stack operations
   if ((s.OpcodeDef->AllowedPrefixes & 2) && s.Prefixes[1]) 
      s.Warnings |= 0x40000; // Stack operation has address size prefix

   // Check for undocumented opcode
   if ((s.OpcodeDef->InstructionFormat & 0x4000) && s.OpcodeDef->Name)
      s.Warnings |= 0x100000; // Undocumented opcode

   // Check for future opcode
   if (s.OpcodeDef->InstructionFormat & 0x2000)
      s.Warnings |= 0x200000; // Opcode reserved for future extensions

   // Check operand size for stack operations
   if ((s.OpcodeDef->AllowedPrefixes & 0x102) == 0x102) {
      if (s.Prefixes[4] == 0x66 || (Get<uint8>(s.OpcodeStart1) == 0xCF && s.OperandSize != WordSize)) {
         s.Warnings |= 0x4000000; // Non-default size for stack operation
      }
   }

   // Check if function ends with ret or unconditional jump (or nop)
   if (IEnd == FunctionEnd && !(s.OpcodeDef->Options & 0x50)) {
      s.Warnings |= 0x8000000; // Function does not end with return or jump
   }

   // Check for multi-byte NOP and UD2
   if (s.OpcodeDef->Options & 0x50) CheckForNops();

   // Check for inaccessible code
   if (IBegin == LabelInaccessible) {
      s.Warnings |= 0x10000000; // Inaccessible code other than NOP or UD2
   }
}


void CDisassembler::FindErrors() {
   // Find any errors in code
   if (IEnd - IBegin > 15) {
      // Instruction longer than 15 bytes
      s.Errors |= 1;
   }
   if (s.Prefixes[2] && (!(s.OpcodeDef->AllowedPrefixes & 0x10) || !(s.MFlags & 1))) {
      // Lock prefix not allowed for this instruction
      s.Errors |= 2;
   }
   if ( s.OpcodeDef->InstructionFormat == 0 
   || ((s.OpcodeDef->InstructionFormat & 0x4000) && s.OpcodeDef->Name == 0)) {
      // Illegal instruction
      s.Errors |= 4;
   }
   if ((s.OpcodeDef->InstructionSet & 0x8000) && WordSize == 64) {
      // Instruction not allowed in 64 bit mode
      s.Errors |= 0x40;
   }
   if (IEnd > LabelEnd && IBegin < LabelEnd) {
      // Instruction crosses a label
      s.Errors |= 0x80;
      // Put interpretation in phase with label
      IEnd = LabelEnd;
   }
   if ((s.MFlags & 3) == 3 && (s.Prefixes[7] & 1) && s.BaseReg == 0 && s.AddressFieldSize == 4) {
      // Attempt to use R13 as base register without displacement
      s.Errors |= 0x200;
   }
   if (LabelEnd >= s.OpcodeStart2+2 && (
         Get<uint16>(s.OpcodeStart2) == 0 
      || Get<uint16>(s.OpcodeStart2) == 0xFFFF
      // || Get<uint16>(s.OpcodeStart2) == 0xCCCC
      )) {
      // Two consecutive bytes of zero gives the instruction: add byte ptr [eax],al
      // This instruction is very unlikely to occur in normal code but occurs
      // frequently in data. Mark to code as probably data.
      // Two bytes of 0xFF makes no legal instruction but occurs frequently in data.
      // Two bytes of 0xCC is debug breaks used by debuggers for marking illegal addresses or unitialized data
      s.Errors = 0x4000;
   }
   if (s.Errors) {
      // Errors found. May be data in code segment
      CountErrors++;
      MarkCodeAsDubious();
   }
}

void CDisassembler::FindRelocations() {
   // Find any relocation sources in this instruction
   SARelocation rel1, rel2;                      // Make relocation records for searching
   rel1.Section = Section;
   rel1.Offset  = IBegin;                        // rel1 marks begin of this instruction
   rel2.Section = Section;
   rel2.Offset  = IEnd;                          // rel2 marks end of this instruction

   // Search for relocations in this instruction
   uint32 irel = Relocations.FindFirst(rel1);    // Finds first relocation source >= IBegin

   if (irel == 0 || irel >= Relocations.GetNumEntries()) {
      // No relocations found
      return;
   }
   if (Relocations[irel] < rel2) {
      // Found relocation points between IBegin and IEnd
      if (Relocations[irel].Offset == s.AddressField && s.AddressFieldSize) {
         // Relocation points to address field
         s.AddressRelocation = irel;
         if (Relocations[irel].Size > s.AddressFieldSize) {
            // Right place but wrong size
            s.Errors |= 0x1000;
         }
      }
      else if (Relocations[irel].Offset == s.ImmediateField && s.ImmediateFieldSize) {
         // Relocation points to immediate operand/jump address field
         s.ImmediateRelocation = irel;
         if (Relocations[irel].Size > s.ImmediateFieldSize) {
            // Right place but wrong size
            s.Errors |= 0x1000;
         }
      }
      else {
         // Relocation source points to a wrong address
         s.Errors |= 0x1000;
      }
      if (s.AddressRelocation) {
         // Found relocation for address field, there may be
         // a second relocation for the immediate field
         if (irel + 1 < Relocations.GetNumEntries() && Relocations[irel+1] < rel2) {
            // Second relocation found
            if (Relocations[irel+1].Offset == s.ImmediateField && s.ImmediateFieldSize) {
               // Relocation points to immediate operand/jump address field
               s.ImmediateRelocation = irel + 1;
               if (Relocations[irel+1].Size > s.ImmediateFieldSize) {
                  // Right place but wrong size
                  s.Errors |= 0x1000;
               }
               else {
                  // Second relocation accepted
                  irel++;
               }
            }
         }
      }
      // Check if there are more relocations
      if (irel + 1 < Relocations.GetNumEntries() && Relocations[irel+1] < rel2) {
         // This relocation points before IEnd but doesn't fit any operand or overlaps previous relocation
         if ((s.Operands[0] & 0xFE) == 0x84 && Relocations[irel+1].Offset == s.ImmediateField + s.ImmediateFieldSize - 2) {
            // Fits segment field of far jump/call
            ;
         }
         else {
            // Relocation doesn't fit anywhere
            s.Errors |= 0x1000;
         }
      }
   }
}

void CDisassembler::FindInstructionSet() {
   // Update instruction set

   // Set InstructionSetMax to maximum instruction set
   uint16 InstSet = s.OpcodeDef->InstructionSet;
   if (InstSet == 7 && s.Prefixes[5] == 0x66) {
      // Change MMX to SSE2 if 66 prefix
      InstSet = 0x12;
   }

   if ((InstSet & 0xFF) > InstructionSetMax) {
      InstructionSetMax = InstSet & 0xFF;
   }

   // Set InstructionSetOR to a bitwise OR of all instruction sets encountered
   InstructionSetOR |= InstSet;

   if (s.OpcodeDef->Options & 0x10) {
      FlagPrevious |= 2;
   }
}

void CDisassembler::CheckLabel() {
   // Check if there is a label at instruction, and write it
   // Write begin and end of function

   // Search in symbol table
   uint32 Sym1, Sym2;                            // First and last symbol

   // Find all symbol table entries at this address
   Sym1 = Symbols.FindByAddress(Section, IBegin, &Sym2);

   if (Sym1) {
      // Found at least one symbol
      // Loop for all symbols with same address
      for (uint32 s = Sym1; s <= Sym2; s++) {

         // Check if label has already been written as a function label
         if (!(Symbols[s].Scope & 0x100) && !(Symbols[s].Type & 0x80000000)) {

            // Write label as a private or public code label
            WriteCodeLabel(s);
         }
      }
      // Get symbol type and size
      DataType = Symbols[Sym2].Type;
      DataSize = GetDataItemSize(DataType);
   }
}

void CDisassembler::CheckForNops() {
   // Check for multi-byte NOP and UD2 instructions

   switch (Opcodei) {
   case 0x3C00: case 0x3C01: case 0x3C02: case 0x11F:  // NOP      
      // These opcodes are intended for NOPs. Indicate if longer than one byte
      if (IEnd - IBegin > 1) s.Warnings |= 0x1000000;
      // Remember NOP
      FlagPrevious |= 1;
      break;

   case 0x8D:   // LEA
      // LEA is often used as NOP with destination = base register
      if (s.Mod < 3 && s.Reg+1 == s.BaseReg && s.IndexReg == 0 && 
      s.AddressSize == s.OperandSize && s.OperandSize >= WordSize) {
         // Destination is same as base register. 
         // Check if displacement is 0
         switch (s.AddressFieldSize) {
         case 0:
            break;
         case 1:
            if (Get<int8>(s.AddressField) != 0) return;
            break;
         case 2:
            if (Get<int16>(s.AddressField) != 0) return;
            break;
         case 4:
            if (Get<int32>(s.AddressField) != 0) return;
            break;
         default:
            return;
         }
         // Displacement is zero. This is a multi-byte NOP
         s.Warnings |= 0x1000000;
         break;
      }

   case 0x86: case 0x87:  // XCHG
   case 0x88: case 0x89: case 0x8A: case 0x8B:  // MOV
      // Check if source and destination are the same register
      if (s.Mod == 3 && s.Reg == s.RM && s.OperandSize >= WordSize) {
         // Moving a register to itself. This is a NOP
         s.Warnings |= 0x1000000;
      }
      break;
   case 0x10B:  // UD2
      FlagPrevious |= 6;
      break;
   }

   if (s.Warnings & 0x1000000) {
      // A multi-byte NOP is detected. 
      // Remove warnings for longer-than-necessary instruction
      s.Warnings &= ~ 0x873D;
      // Remember NOP
      FlagPrevious |= 1;
   }
}

void CDisassembler::InitialErrorCheck() {
   // Check for illegal relocations table entries
   uint32 i;                                     // Loop counter

   // Loop through relocations table
   for (i = 1; i < Relocations.GetNumEntries(); i++) {
      if (Relocations[i].TargetOldIndex >= Symbols.GetLimit()) {
         // Nonexisting relocation target
         Relocations[i].TargetOldIndex = 0;
      }
      if (Relocations[i].RefOldIndex >= Symbols.GetLimit()) {
         // Nonexisting reference index
         Relocations[i].RefOldIndex = 0;
      }
      // Remember types of relocations in source
      RelocationsInSource |= Relocations[i].Type;
   }

   // Check opcode tables
   if (NumOpcodeTables1 != NumOpcodeTables2) err.submit(9007, 0xFFFF);
}

void CDisassembler::FinalErrorCheck() {
   // Check for illegal entries in symbol table and relocations table
   uint32 i;                                     // Loop counter
   int SpaceWritten = 0;                         // Blank line written

   // Loop through symbol table
   for (i = 1; i < Symbols.GetNumEntries(); i++) {
      if (Symbols[i].Section <= 0 || (Symbols[i].Type & 0x80000000)) {
         // Constant or external symbol or section
         continue;
      }
      if ((uint32)Symbols[i].Section >= Sections.GetNumEntries()
      || Symbols[i].Offset > Sections[Symbols[i].Section].TotalSize) {
         // Symbol has illegal address
         // Blank line
         if (!SpaceWritten++) OutFile.NewLine();
         // Write comment
         OutFile.Put(CommentSeparator);
         OutFile.Put("Error: Symbol ");
         // Write symbol name
         OutFile.Put(Symbols.GetName(i));
         // Write the illegal address
         OutFile.Put(" has a non-existing address. Section: ");
         if (Symbols[i].Section != ASM_SEGMENT_IMGREL) {
            OutFile.PutDecimal(Symbols[i].Section, 1);
         }
         else {
            OutFile.Put("Unknown");
         }
         OutFile.Put(" Offset: ");
         OutFile.PutHex(Symbols[i].Offset, 1);
         OutFile.NewLine();
      }
   }
   // Loop through relocations table
   for (i = 1; i < Relocations.GetNumEntries(); i++) {
      // Check source address
      if (Relocations[i].Section == 0 
      || (uint32)Relocations[i].Section >= Sections.GetNumEntries()
      || (Sections[Relocations[i].Section].Type & 0xFF) == 3
      || Relocations[i].Offset >= Sections[Relocations[i].Section].InitSize) {
         // Relocation has illegal source address
         // Blank line
         if (!SpaceWritten++) OutFile.NewLine();
         // Write comment
         OutFile.Put(CommentSeparator);
         OutFile.Put("Error: Relocation number ");
         OutFile.PutDecimal(i);
         OutFile.Put(" has a non-existing source address. Section: ");
         if (Relocations[i].Section != ASM_SEGMENT_IMGREL) {
            OutFile.PutDecimal(Relocations[i].Section, 1);
         }
         else {
            OutFile.Put("Unknown");
         }
         OutFile.Put(" Offset: ");
         OutFile.PutHex(Relocations[i].Offset, 1);
         OutFile.NewLine();
      }
      // Check target
      if (Relocations[i].TargetOldIndex == 0 
      || Relocations[i].TargetOldIndex >= Symbols.GetLimit()
      || Relocations[i].RefOldIndex >= Symbols.GetLimit()) {
         // Relocation has illegal target
         // Blank line
         if (!SpaceWritten++) OutFile.NewLine();
         // Write comment
         OutFile.Put(CommentSeparator);
         OutFile.Put("Error: Relocation number ");
         OutFile.PutDecimal(i);
         OutFile.Put(" at section ");
         OutFile.PutDecimal(Relocations[i].Section);
         OutFile.Put(" offset ");
         OutFile.PutHex(Relocations[i].Offset);
         OutFile.Put(" has a non-existing target index. Target: ");
         OutFile.PutDecimal(Relocations[i].TargetOldIndex, 1);
         if (Relocations[i].RefOldIndex) {
            OutFile.Put(", Reference point index: ");
            OutFile.PutDecimal(Relocations[i].RefOldIndex, 1);
         }
         OutFile.NewLine();
      }
   }
}

void CDisassembler::CheckNamesValid() {
   // Fix invalid symbol and section names
   uint32 i;                                     // Loop counter

   // Loop through sections
   for (i = 1; i < Sections.GetNumEntries(); i++) {
      char * SecName = NameBuffer.Buf() + Sections[i].Name;
      if (SecName[0] == '.') {
         // Name begins with dot
         // Check for reserved names
         if (stricmp(SecName, ".text") == 0
         ||  stricmp(SecName, ".data") == 0
         ||  stricmp(SecName, ".code") == 0
         ||  stricmp(SecName, ".const") == 0) {
            // Change . to _ in beginning of name to avoid reserved directive name
            SecName[0] = '_'; 
         }
         else {
            // Other name beginning with .
            // Set option dotname
            MasmOptions |= 1;
         }
      }
   }
   // Loop through symbols
   for (i = 1; i < Symbols.GetNumEntries(); i++) {
      if (Symbols[i].Name) {
         char * SymName = Symbols.GetName(i);
         if (SymName[0] == '.') {
            // Name begins with dot
            if (Symbols[i].Type & 0x80000000) {
               // This is a segment. Check for reserved names
               if (stricmp(SymName, ".text") == 0
                  ||  stricmp(SymName, ".data") == 0
                  ||  stricmp(SymName, ".code") == 0
                  ||  stricmp(SymName, ".const") == 0) {
                     // Change . to _ in beginning of name to avoid reserved directive name
                     SymName[0] = '_'; 
                     continue;
                  }
            }         
            // Set option dotname
            MasmOptions |= 1;
         }
      }
   }
}

void CDisassembler::FixRelocationTargetAddresses() {
   // Fix missing relocation target addresses
   // to section:offset addresses
   uint32 r;                                     // Relocation index
   uint32 s;                                     // Symbol index

   // Loop through relocations
   for (r = 1; r < Relocations.GetNumEntries(); r++) {

      if (Relocations[r].TargetOldIndex == 0 && (Relocations[r].Type & 0x60)) {
         // Target symbol not defined. Make new symbol
         SASymbol sym;
         sym.Reset();

         // Find target address from relocation source
         uint8 * pSectionData = Sections[Relocations[r].Section].Start;
         if (!pSectionData) continue;
         int64 TargetOffset = 0;
         if (Relocations[r].Size == 4) {
            TargetOffset = *(int32*)(pSectionData + Relocations[r].Offset);
         }
         else if (Relocations[r].Size == 8) {
            TargetOffset = *(int64*)(pSectionData + Relocations[r].Offset);
         }
         else {
            // Error: wrong size
            continue;
         }
         if (HighDWord(TargetOffset)) {
            // Error: out of range
            continue;
         }
         // Translate to section:offset address
         if (!(TranslateAbsAddress(TargetOffset, sym.Section, sym.Offset))) {
            // Translation failed
            continue;
         }
         // Default scope is file local
         sym.Scope = 2;

         // Add symbol if it doesn't exist or get index of existing symbol
         s = Symbols.NewSymbol(sym);

         // Make reference to symbol from relocation record
         if (s) {
            Relocations[r].TargetOldIndex = Symbols[s].OldIndex;
         }
      }
   }
}

int CDisassembler::TranslateAbsAddress(int64 Addr, int32 &Sect, uint32 &Offset) {
   // Translate absolute virtual address to section and offset
   // Returns 1 if valid address found.
   int32 Section;

   // Get image-relative address
   Addr -= ImageBase;
   // Fail if too big
   if (HighDWord(Addr)) return 0;

   // Search through sections
   for (Section = 1; (uint32)Section < Sections.GetNumEntries(); Section++) {
      uint32 SectionAddress = Sections[Section].SectionAddress;
      if ((uint32)Addr >= SectionAddress && (uint32)Addr < SectionAddress + Sections[Section].TotalSize) {
         // Address is within this section
         // Return section and offset
         Sect = Section;
         Offset = (uint32)Addr - SectionAddress;
         // Return 1 to indicate success
         return 1;
      }
   }
   // Not found. Return 0
   return 0;
}

uint32 CDisassembler::GetDataItemSize(uint32 Type) {
   // Get size in bytes of data item with specified type
   uint32 Size = 1;

   switch (Type & 0xFF) {
   case 1:
      Size = 1;  break;
   case 2:
      Size = 2;  break;
   case 3: case 0x43: case 0x4B:
      Size = 4;  break;
   case 4: case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: 
   case 0x44: case 0x4C: case 0x5B:
      Size = 8;  break;
   case 5: case 0x45:
      Size = 10;  break;
   case 7:
      Size = 6;  break;
   case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: 
   case 0x68: case 0x6B: case 0x6C:
      Size = 16;  break;
   case 0x0B: case 0x0C:
      // Function pointer
      Size = WordSize / 8;  break;
   case 0x0D:
      // Far function pointer
      Size = WordSize / 8 + 2;  break;
   }

   return Size;
}

uint32 CDisassembler::GetDataElementSize(uint32 Type) {
   // Get size of vector element in data item with specified type
   return GetDataItemSize(Type & 7);
}

int32 CDisassembler::GetSegmentRegisterFromPrefix() {
   // Translate segment prefix to segment register
   switch (s.Prefixes[0]) {
   case 0x26:  // ES:
      return 0;
   case 0x2E:  // CS:
      return 1;
   case 0x36:  // SS:
      return 2;
   case 0x3E:  // DS:
      return 3;
   case 0x64:  // FS:
      return 4;
   case 0x65:  // GS:
      return 5;
   }
   return -1;  // Error: none
}
