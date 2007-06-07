/****************************  elf2mac.cpp   *********************************
* Author:        Agner Fog
* Date created:  2007-01-10
* Last modified: 2007-04-26
* Project:       objconv
* Module:        elf2mac.cpp
* Description:
* Module for converting ELF file to Mach-O file
*
* (c) 2007 GNU General Public License www.gnu.org/copyleft/gpl.html
*****************************************************************************/

#include "stdafx.h"

CELF2MAC::CELF2MAC () {
   // Constructor
   memset(this, 0, sizeof(*this));                    // Reset everything
}
CELF2MAC::~CELF2MAC () {
}
void CELF2MAC::Convert() {
   // Do the conversion
   // Some compilers require this-> for accessing members of template base class,
   // according to the so-called two-phase lookup rule.
   //this->GetFileType();                               // Determine file type and word size
   if (this->WordSize != 32) err.submit(2012, this->WordSize, 32);// Check word size
   //this->ParseFile();                                 // Initialize file interpreter
   // Call the subfunctions
   ToFile.SetFileType(FILETYPE_MACHO_LE);             // Set type of new file
   MakeFileHeader();                                  // Make file header
   MakeSectionsIndex();                               // Make sections index translation table
   MakeSymbolTable();                                 // Make symbol table and string tables
   MakeSections();                                    // Make sections and relocation tables
   //HideUnusedSymbols();                             // Hide unused symbols
   MakeBinaryFile();                                  // Put sections together
   *this << ToFile;                                   // Take over new file buffer
}
void CELF2MAC::MakeFileHeader() {
   // Convert subfunction: Make file header and load segment command
   MAC_header NewHeader;                              // new file header
   NewHeader.magic      = MAC_MAGIC_32;		         // mach magic number identifier
   NewHeader.cputype    = MAC_CPU_TYPE_I386;
   NewHeader.cpusubtype = MAC_CPU_SUBTYPE_I386_ALL;
   NewHeader.filetype   = MAC_OBJECT;
   NewHeader.ncmds      = 3;                          // Three commands = segment, symbol table, dynsymtab
   NewHeader.sizeofcmds = 0;                          // Set this later
   NewHeader.flags      = 0;                          // 

   // put file header in OutFile
   ToFile.Push(&NewHeader, sizeof(NewHeader));
}

void CELF2MAC::MakeSectionsIndex() {
   // Make sections index translation table and section offset table.
   // We must make these tables before the sections, because they are needed for the
   // symbol tables and relocation tables, and we must make the symbol tables before 
   // the relocation tables, and we must make the relocation tables together with the
   // sections. 
   uint32 oldsec;                         // Section number in old file
   uint32 newsec = 0;                     // Section number in new file
   NewSectIndex. SetNum(this->NSections); // Allocate size for section index table
   NewSectIndex. SetZero();               // Initialize
   NewSectOffset.SetNum(this->NSections); // Allocate buffer for section offset table
   NewSectOffset.SetZero();               // Initialize

   uint32 NewVirtualAddres = 0;           // Virtual address of new section  as specified in Mach-O file

   // First loop through old sections
   for (oldsec = 0; oldsec < this->NSections; oldsec++) {
      NewSectIndex[oldsec]  = 0;
      NewSectOffset[oldsec] = 0;

      // Get section name
      char * sname = "";
      uint32 namei = this->SectionHeaders[oldsec].sh_name;
      if (namei >= this->SecStringTableLen) {
         err.submit(2112);
      }
      else sname = this->SecStringTable + namei;

      if (cmd.DebugInfo == CMDL_DEBUG_STRIP) {
         // Check for debug section names
         if (strncmp(sname, ".note",    5) == 0
         ||  strncmp(sname, ".comment", 8) == 0
         ||  strncmp(sname, ".stab",    5) == 0
         ||  strncmp(sname, ".debug",   6) == 0) {
            // Remove this section
            this->SectionHeaders[oldsec].sh_type = SHT_REMOVE_ME;
            cmd.CountDebugRemoved();
            continue;
         }
      }

      if (cmd.ExeptionInfo == CMDL_EXCEPTION_STRIP) {
         // Check for exception section name
         if (strncmp(sname, ".eh_frame", 9) == 0) {
            // Remove this section
            this->SectionHeaders[oldsec].sh_type = SHT_REMOVE_ME;
            cmd.CountExceptionRemoved();
            continue;
         }
      }

      // Search for program data sections only
      if (this->SectionHeaders[oldsec].sh_type != SHT_PROGBITS 
      &&  this->SectionHeaders[oldsec].sh_type != SHT_NOBITS) {
         // Has no data. Ignore
         continue;
      }

      // Section index translation table
      NewSectIndex[oldsec] = newsec++;

      // Calculate virtual memory address of section. This address does not have 
      // much to do with the final address, but it is needed in relocation entries.

      // Alignment
      int NewAlign = FloorLog2(this->SectionHeaders[oldsec].sh_addralign);
      if (NewAlign > 12) NewAlign = 12;   // What is the limit for highest alignment?
      int AlignBy = 1 << NewAlign;

      // Align memory address
      NewVirtualAddres = (NewVirtualAddres + AlignBy - 1) & -AlignBy;

      // Virtual memory address of new section
      NewSectOffset[oldsec] = NewVirtualAddres;

      // Increment memory address
      NewVirtualAddres += this->SectionHeaders[oldsec].sh_size;
   }
   // Store number of sections in new file
   NumSectionsNew = newsec;

   // Calculate file offset of first raw data
   RawDataOffset = sizeof(MAC_header) 
      + sizeof(MAC_segment_command) 
      + NumSectionsNew * sizeof(MAC_section)
      + sizeof(MAC_symtab_command) 
      + sizeof(MAC_dysymtab_command);

   // Align end of memory address by 4
   NewVirtualAddres = (NewVirtualAddres + 3) & uint32(-4);

   // Make segment command
   MAC_segment_command NewSegment;
   memset(&NewSegment, 0, sizeof(NewSegment));
   NewSegment.cmd      = MAC_LC_SEGMENT;
   NewSegment.cmdsize  = sizeof(MAC_segment_command) + NumSectionsNew * sizeof(MAC_section);
   NewSegment.fileoff  = RawDataOffset;
   NewSegment.nsects   = NumSectionsNew;
   NewSegment.maxprot  = NewSegment.initprot = 7; // 1=read, 2=write, 4=execute
   NewSegment.vmsize   = NewVirtualAddres;
   NewSegment.filesize = 0;                       // Changed later

   // put segment command in OutFile
   CommandOffset = ToFile.Push(&NewSegment, sizeof(NewSegment));
}

void CELF2MAC::MakeSymbolTable() {
   // Convert subfunction: Symbol table and string tables
   uint32 oldsec;                  // Section number in old file
   Elf32_Shdr OldHeader;           // Old section header
   int FoundSymTab = 0;            // Found symbol table
   int8 * strtab;                  // Old symbol string table
   int8 * symtab;                  // Old symbol table
   uint32 symtabsize;              // Size of old symbol table
   int8 * symtabend;               // End of old symbol table
   int entrysize;                  // Size of each entry in old symbol table
   Elf32_Sym OldSym;               // Old symbol table record
   uint32 OldSymI;                 // Symbol index in old symbol table
   char * symname;                 // Symbol name
   int NewSection = 0;             // New section index
   int NewType;                    // New symbol type
   int NewDesc;                    // New symbol reference type
   int Value;                      // Symbol value
   uint32 Scope;                   // 0: Local, 1: Public, 2: External 
   // Loop through old sections to find symbol table
   for (oldsec = 0; oldsec < this->NSections; oldsec++) {

      // Search for program data sections only
      if (this->SectionHeaders[oldsec].sh_type == SHT_SYMTAB 
      ||  this->SectionHeaders[oldsec].sh_type == SHT_DYNSYM) {
         FoundSymTab++;

         // Copy symbol table header for convenience
         OldHeader = this->SectionHeaders[oldsec];

         // Find associated string table
         if (OldHeader.sh_link >= (uint32)(this->NSections)) {
            err.submit(2035); OldHeader.sh_link = 0;
         }
         strtab = this->Buf() + this->SectionHeaders[OldHeader.sh_link].sh_offset;

         // Find old symbol table
         entrysize = OldHeader.sh_entsize;
         if (entrysize < sizeof(Elf32_Sym)) {err.submit(2033); entrysize = sizeof(Elf32_Sym);}

         symtab = this->Buf() + OldHeader.sh_offset;
         symtabsize = OldHeader.sh_size;
         symtabend = symtab + symtabsize;

         // Allocate table OldSymbolScope
         NumOldSymbols = symtabsize / entrysize;
         if (NumOldSymbols > 0 && NumOldSymbols < 1000000) {
            OldSymbolScope.SetNum(NumOldSymbols);
            OldSymbolScope.SetZero();
         }
         // Loop through old symbol table
         for (OldSymI = 0; symtab < symtabend; symtab += entrysize, OldSymI++) {

            if (OldSymI == 0) continue; // First symbol entry in ELF file is unused

            // Copy 32 bit symbol table entry or convert 64 bit entry
            OldSym = *(Elf32_Sym*)symtab;
 
            // Old symbol type
            int type = OldSym.st_type;

            // Old symbol storage class = binding
            int binding = OldSym.st_bind;

            // Get symbol name
            if (OldSym.st_name < symtabsize) {
               symname = strtab + OldSym.st_name;
            }
            else {
               continue;       // Ignore
            }
            if (!symname) {
               // No name
               symname = "";
            }
            
            NewType = NewDesc = 0; // New symbol type
            // Value = address
            Value = OldSym.st_value;

            // Section
            if (OldSym.st_shndx == SHN_UNDEF) {
               NewSection = 0; // External
            }
            else if ((int16)(OldSym.st_shndx) == SHN_ABS) {
               NewType |= MAC_N_ABS; // Absolute symbol
               NewDesc |= MAC_N_NO_DEAD_STRIP;
               NewSection = 0; 
            }
            else if (OldSym.st_shndx >= this->NSections) {
               err.submit(2036, OldSym.st_shndx); // Special/unknown section index or out of range
            }
            else {
               // Normal section index. 
               // Look up in section index translation table and add 1 because it is 1-based
               NewSection = NewSectIndex[OldSym.st_shndx] + 1;
               // Value must be absolute address. Add section address
               Value += NewSectOffset[OldSym.st_shndx];
            }

            // Convert binding/storage class
            switch (binding) {
            case STB_LOCAL:   // Local
               Scope = S_LOCAL;
               if (!(NewType & MAC_N_ABS)) NewType |= MAC_N_SECT;
               break;

            case STB_GLOBAL:
               if (NewSection || (NewType & MAC_N_ABS)) {
                  // Public
                  Scope = S_PUBLIC;
                  NewType |= MAC_N_EXT;
                  if (!(NewType & MAC_N_ABS)) NewType |= MAC_N_SECT;
               }
               else {
                  // External
                  Scope = S_EXTERNAL;
                  NewType |= MAC_N_EXT;
               }
               NewDesc |= MAC_REF_FLAG_UNDEFINED_NON_LAZY;
               break;

            case STB_WEAK:
               if (NewSection) {
                  // Weak public
                  Scope = S_PUBLIC;
                  NewType |= MAC_N_EXT | MAC_N_SECT;
                  NewDesc |= MAC_N_WEAK_DEF;
                  err.submit(1051, symname);  // Weak public only allowed in coalesced section
               }
               else {
                  // Weak external
                  Scope = S_EXTERNAL;
                  NewType |= MAC_N_EXT;
                  NewDesc |= MAC_N_WEAK_REF;
               }
               break;

            default:
               Scope = S_LOCAL;
               err.submit(2037, binding); // Other. Not supported
            }

            // Make record depending on type
            switch (type) {
            case STT_OBJECT: case STT_NOTYPE:
               // Data object
               break;

            case STT_FUNC:
               // Function
               break;

            case STT_FILE: 
               // File name record. Ignore
               continue;

            case STT_SECTION:
               // Section name record. (Has no name)
               break;

            case STT_COMMON:
            default:
               err.submit(2038, type); // Symbol type not supported
               continue;
            }

            // Store new symbol record in the appropriate table
            if (Scope > 2) err.submit(9000);
            NewSymTab[Scope].AddSymbol(OldSymI, symname, NewType, NewDesc, NewSection, Value);

            // Store scope in OldSymbolScope
            if (OldSymI < NumOldSymbols) {
               OldSymbolScope[OldSymI] = Scope;
            }
         } // End OldSymI loop
      }
   } // End search for symbol table
   if (FoundSymTab == 0) err.submit(2034); // Symbol table not found
   if (FoundSymTab  > 1) err.submit(1032); // More than one symbol table found
}

void CELF2MAC::MakeSections() {
   // Convert subfunction: Make sections and relocation tables
   uint32 oldsec;                  // Section number in old file
   uint32 relsec;                  // Relocation section in old file
   MAC_section NewHeader;          // New section header
   Elf32_Shdr OldHeader;           // Old section header
   Elf32_Shdr OldRelHeader;        // Old relocation section header
   Elf32_Rela OldRelocation;       // Old relocation table entry
   uint32 NewVirtualAddres = 0;    // Virtual address of new section
   uint32 NewRawDataOffset = 0;    // Offset into NewRawData of section. 
   // NewRawDataOffset is different from NewVirtualAddres if alignment of sections in 
   // the object file is different from alignment of sections in memory

   uint32 NumSymbols[4];           // Accumulated number of entries in each NewSymTab[]
   NumSymbols[0] = 0;
   NumSymbols[1] = NumSymbols[0] + NewSymTab[0].GetNumEntries();
   NumSymbols[2] = NumSymbols[1] + NewSymTab[1].GetNumEntries();
   NumSymbols[3] = NumSymbols[2] + NewSymTab[2].GetNumEntries();

   NewSectHeadOffset = ToFile.GetDataSize();

   // Second loop through old sections
   for (oldsec = 0; oldsec < this->NSections; oldsec++) {

      // Copy old header for convenience
      OldHeader = this->SectionHeaders[oldsec];

      // Search for program data sections only
      if (OldHeader.sh_type == SHT_PROGBITS || OldHeader.sh_type == SHT_NOBITS) {

         // Reset new section header
         memset(&NewHeader, 0, sizeof(NewHeader));

         // Section name
         char * sname = "";
         uint32 namei = OldHeader.sh_name;
         if (namei >= this->SecStringTableLen) err.submit(2112);
         else sname = this->SecStringTable + namei;

         // Translate section name and truncate to 16 characters
         if (!stricmp(sname,".text") || !stricmp(sname,"_text")) {
            strcpy(NewHeader.sectname, "__text");
            strcpy(NewHeader.segname,  "__TEXT");
         }
         else if (!stricmp(sname,".data") || !stricmp(sname,"_data")) {
            strcpy(NewHeader.sectname, "__data");
            strcpy(NewHeader.segname,  "__DATA");
         }
         else if (!strnicmp(sname+1,"bss", 3)) {
            strcpy(NewHeader.sectname, "__bss");
            strcpy(NewHeader.segname,  "__DATA");
         }
         else if (!strnicmp(sname+1,"const", 5)) {
            strcpy(NewHeader.sectname, "__const");
            strcpy(NewHeader.segname,  "__DATA");
         }
         else if (OldHeader.sh_flags & SHF_EXECINSTR) {
            // Other code section
            if (strlen(NewHeader.sectname) > 16) err.submit(1040, NewHeader.sectname); // Warning: name truncated
            strncpy(NewHeader.sectname, sname, 16);
            strcpy(NewHeader.segname,  "__TEXT");
         }
         else {
            // Other data section. Truncate name to 16 characters
            if (strlen(NewHeader.sectname) > 16) err.submit(1040, NewHeader.sectname); // Warning: name truncated
            strncpy(NewHeader.sectname, sname, 16);
            strcpy(NewHeader.segname,  "__DATA");
         }
         // Raw data
         NewHeader.size = OldHeader.sh_size;  // section size in file
         if (OldHeader.sh_size) {
            // File ptr to raw data for section
            NewHeader.offset = NewRawData.GetDataSize() + RawDataOffset;

            if (OldHeader.sh_type != SHT_NOBITS) { // Not for .bss segment
               // Copy raw data
               NewRawDataOffset = NewRawData.Push(this->Buf()+OldHeader.sh_offset, OldHeader.sh_size); 
               NewRawData.Align(4);
            }
         }

         // Section flags
         if (OldHeader.sh_flags & SHF_EXECINSTR) {
            NewHeader.flags |= MAC_S_ATTR_PURE_INSTRUCTIONS | MAC_S_ATTR_SOME_INSTRUCTIONS;
         }
         else if (OldHeader.sh_type == SHT_NOBITS) {
            NewHeader.flags |= MAC_S_ZEROFILL;              // .bss segment
         }

         // Alignment
         NewHeader.align = FloorLog2(OldHeader.sh_addralign);
         if (NewHeader.align > 12) NewHeader.align = 12;   // What is the limit for highest alignment?
         int AlignBy = 1 << NewHeader.align;

         // Align memory address
         NewVirtualAddres = (NewVirtualAddres + AlignBy - 1) & -AlignBy;

         // Virtual memory address of new section 
         NewHeader.addr = NewVirtualAddres; 
         NewVirtualAddres += OldHeader.sh_size;

         // Find relocation table for this section by searching through all sections
         for (relsec = 1; relsec < this->NSections; relsec++) {

            // Get section header
            OldRelHeader = this->SectionHeaders[relsec];            
            
            // Check if this is a relocations section referring to oldsec
            if ((OldRelHeader.sh_type == SHT_REL || OldRelHeader.sh_type == SHT_RELA) // if section is relocation
            && OldRelHeader.sh_info == oldsec) { // and if section refers to current section

               // Relocation table found. Get pointer
               int8 * reltab = this->Buf() + OldRelHeader.sh_offset;
               int8 * reltabend = reltab + OldRelHeader.sh_size;

               // Get entry size
               int entrysize = OldRelHeader.sh_entsize;
               int expectedentrysize = (OldRelHeader.sh_type == SHT_REL) ?
                  sizeof(Elf32_Rel) : sizeof(Elf32_Rela);
               if (entrysize < expectedentrysize) {err.submit(2033); entrysize = expectedentrysize;}

               // File pointer to relocations
               NewHeader.reloff = NewRelocationTab.GetNumEntries()*sizeof(MAC_relocation_info);  // Offset to first relocation table added later
               // Loop through relocation table entries
               for (; reltab < reltabend; reltab += entrysize) {

                  // Copy relocation entry with or without addend
                  OldRelocation.r_addend = 0;
                  memcpy(&OldRelocation, reltab, entrysize);
                  // Find inline addend
                  uint32 InlinePosition = (uint32)(NewRawDataOffset + OldRelocation.r_offset);

                  // Check that address is valid
                  if (InlinePosition >= GetDataSize()) {
                     // Address is invalid
                     err.submit(2032);
                     break;
                  }

                  // Pointer to inline addend
                  int32 * piaddend = (int32*)(NewRawData.Buf() + InlinePosition);

                  // Add old addend if any
                  *piaddend += OldRelocation.r_addend;

                  // Define relocation parameters
                  uint32  r_address = 0;      // section-relative offset to relocation source
                  uint32  r_symbolnum = 0;    // symbol index if r_extern == 1 or section ordinal if r_extern == 0
                  uint32  r_value = 0;        // value of relocation target
                  int     r_scattered = 0;    // use scattered relocation
                  int     r_pcrel = 0;        // self relative
                  int     r_length = 2;       // size of source: 0=byte, 1=word, 2=long
                  int     r_extern = 0;       // public or external
                  int     r_type = 0;         // if not 0, machine specific relocation type
                  int     Scope = 0;          // Symbol scope: 0 = local, 1 = public, 2 = external
                  // source offset
                  r_address = OldRelocation.r_offset;

                  // target scope
                  if (OldRelocation.r_sym < NumOldSymbols) {
                     Scope = OldSymbolScope[OldRelocation.r_sym];
                  }

                  // Get r_extern: 0 = local target referenced by address,
                  //               1 = public or external symbol referenced by symbol table index
                  switch (Scope) {
                  case S_LOCAL:  // Local target must be referenced by address
                     r_extern = 0;  break;

                  case S_PUBLIC:  // Public target is optionally referenced by index or by address
                     r_extern = 0;
                     // r_extern = 1; is probably not allowed
                     break;

                  case S_EXTERNAL:  // External target is always referenced by index
                     r_extern = 1;  break;
                  }

                  // Get zero-based index into NewSymTab[Scope]
                  int newindex = NewSymTab[Scope].TranslateIndex(OldRelocation.r_sym);
                  if (newindex < 0) {
                     // Symbol not found or wrong type
                     err.submit(2031); 
                     break;
                  }
                  if (r_extern) {
                     // r_symbolnum is zero based index into combined symbol tables.
                     // Add number of entries in preceding NewSymTab tables to index 
                     // into NewSymTab[Scope]
                     r_symbolnum = newindex + NumSymbols[Scope];
                  }
                  else {
                     // r_extern = 0. r_symbolnum = target section
                     r_symbolnum = NewSymTab[Scope][newindex].n_sect;
                     // Absolute address of target stored inline in source
                     *piaddend  += NewSymTab[Scope][newindex].n_value;
                  }

                  // New symbol offset to segment, not to section
                  int NewSymbolSegmentOffset = 0;
                  if (oldsec < this->NSections) {
                     NewSymbolSegmentOffset = NewSectOffset[oldsec] + OldRelocation.r_offset;
                  }

                  // Get relocation type and fix addend
                  if (this->WordSize == 32) {
                     switch(OldRelocation.r_type) {
                     case R_386_NONE:    // Ignored
                        continue;

                     case R_386_32:      // 32-bit absolute virtual address
                        r_type  = MAC_RELOC_VANILLA;  
                        break;

                     case R_386_PC32:   // 32-bit self-relative
                        r_type  = MAC_RELOC_VANILLA;  
                        r_pcrel = 1;
                        // Mach-O format requires that self-relative addresses must have
                        // self-relative values already before relocation. Therefore
                        // the source address is subtracted.
                        // (The PC reference point is the end of the source = start
                        // of source + 4, but ELF files have the same offset so no further
                        // correction is needed when converting from ELF file).
                       *piaddend -= r_address + NewHeader.addr;
                        break;

                     case R_386_GOT32: case R_386_GLOB_DAT: case R_386_GOTOFF: case R_386_GOTPC:
                        // Global offset table
                        err.submit(2042);     // cannot convert position-independent code
                        err.ClearError(2042); // report this error only once
                        r_type = 0;
                        break;

                     case R_386_PLT32: case R_386_JMP_SLOT: 
                        // procedure linkage table
                        err.submit(2043);     // cannot convert import table
                        err.ClearError(2043); // report this error only once
                        r_type = 0;
                        break;

                     default:      // Unknown or unsupported relocation method
                        err.submit(2030, OldRelocation.r_type); 
                        r_type = 0;  break;
                     }
                  }
                  else { // WordSize == 64
                     err.submit(2030, OldRelocation.r_type); 
                  }
                  if (!r_pcrel) {
                     // Warn for position dependent code. 
                     // This warning is currently turned off in error.cpp.
                     err.submit(1050, this->SymbolName(OldRelocation.r_sym)); 
                     // Write this error only once
                     err.ClearError(1050);
                  }

                  // Make relocation entry
                  MAC_relocation_info rel;
                  memset(&rel, 0, sizeof(rel));

                  // Make non-scattered relocation entry
                  rel.r_address   = r_address; 
                  rel.r_symbolnum = r_symbolnum; 
                  rel.r_pcrel     = r_pcrel; 
                  rel.r_length    = r_length; 
                  rel.r_extern    = r_extern; 
                  rel.r_type      = r_type; 

                  // Store relocation entry
                  NewRelocationTab.Push(&rel, sizeof(rel));
                  NewHeader.nreloc++;

                  // Remember that symbol is used
                  /*
                  if (SymbolsUsed && OldRelocation.r_type && NewRelocation.r_symbolnum < ?) {
                  SymbolsUsed[NewRelocation.r_symbolnum]++;
                  } */                                 

               } // End of relocations loop

            } // End of if right relocation table

         } // End of search for relocation table

         // Align raw data for next section
         NewRawData.Align(4);

         // Store section header in file
         ToFile.Push(&NewHeader, sizeof(NewHeader));

      } // End of if section has program data

   } // End of loop through old sections

} // End of function MakeSections

/*
void CELF2MAC::HideUnusedSymbols() {
// Hide unused symbols if stripping debug info or exception info
} */

void CELF2MAC::MakeBinaryFile() {
   // Convert subfunction: Putting sections together
   // File header, segment command and section headers have been inserted.
   int i;

   // Update segment header for segment size in file
   ((MAC_segment_command*)(ToFile.Buf()+CommandOffset))->filesize = NewRawData.GetDataSize();

   // Make symbol table command
   MAC_symtab_command symtab;
   memset(&symtab, 0, sizeof(symtab));
   symtab.cmd = MAC_LC_SYMTAB;
   symtab.cmdsize = sizeof(symtab);
   // symoff, nsyms, stroff, strsize inserted later
   // Store symtab command
   NewSymtabOffset = ToFile.Push(&symtab, sizeof(symtab));

   // Make MAC_dysymtab_command command
   MAC_dysymtab_command dysymtab;
   memset(&dysymtab, 0, sizeof(dysymtab));
   dysymtab.cmd        = MAC_LC_DYSYMTAB;
   dysymtab.cmdsize    = sizeof(dysymtab);
   dysymtab.ilocalsym  = 0;                            // index to local symbols
   dysymtab.nlocalsym  = NewSymTab[0].GetNumEntries(); // number of local symbols 
   dysymtab.iextdefsym = dysymtab.nlocalsym;           // index to externally defined symbols
   dysymtab.nextdefsym = NewSymTab[1].GetNumEntries(); // number of externally defined symbols 
   dysymtab.iundefsym  = dysymtab.iextdefsym + dysymtab.nextdefsym;	// index to public symbols
   dysymtab.nundefsym  = NewSymTab[2].GetNumEntries(); // number of public symbols
   // Store MAC_dysymtab_command command
   ToFile.Push(&dysymtab, sizeof(dysymtab));

   // Store section data
   uint32 Current = ToFile.Push(NewRawData.Buf(), NewRawData.GetDataSize());
   if (Current != RawDataOffset) err.submit(9000);

   ToFile.Align(4);

   // Store relocation tables
   uint32 Reltabs = ToFile.Push(NewRelocationTab.Buf(), NewRelocationTab.GetDataSize());

   // Initialize new string table. First string is empty
   NewStringTable.Push(0, 1);

   // Store symbol tables and make string table
   uint32 Symtabs = ToFile.GetDataSize();
   uint32 NumSyms = 0;
   for (i = 0; i < 3; i++) {
      NumSyms += NewSymTab[i].GetNumEntries();
      NewSymTab[i].StoreList(&ToFile, &NewStringTable);
   }

   // Store string table
   uint32 StringTab = ToFile.Push(NewStringTable.Buf(), NewStringTable.GetDataSize());

   // Set missing values in file header
   ((MAC_header*)ToFile.Buf())->sizeofcmds = RawDataOffset - sizeof(MAC_header);

   // Adjust relocation offsets in section headers
   MAC_section* sectp = (MAC_section*)(ToFile.Buf() + NewSectHeadOffset);
   for (i = 0; i < NumSectionsNew; i++, sectp++) {
      sectp->reloff += Reltabs;
   }

   // Set missing symoff, nsyms, stroff, strsize in symtab command
   MAC_symtab_command * symtabp = (MAC_symtab_command*)(ToFile.Buf() + NewSymtabOffset);
   symtabp->symoff  = Symtabs;
   symtabp->nsyms   = NumSyms;
   symtabp->stroff  = StringTab;
   symtabp->strsize = NewStringTable.GetDataSize();
}
