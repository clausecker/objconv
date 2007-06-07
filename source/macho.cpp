/****************************    macho.cpp    *******************************
* Author:        Agner Fog
* Date created:  2007-01-06
* Last modified: 2007-01-06
* Project:       objconv
* Module:        macho.cpp
* Description:
* Module for reading Mach-O files
*
* Class CMACHO is used for reading, interpreting and dumping Mach-O files.
*
* (c) 2007 GNU General Public License www.gnu.org/copyleft/gpl.html
*****************************************************************************/
#include "stdafx.h"

// Machine names
SIntTxt MacMachineNames[] = {
   {MAC_CPU_TYPE_I386,      "Intel 32 bit"},
   {MAC_CPU_TYPE_X86_64,    "Intel 64 bit"},
   {MAC_CPU_TYPE_ARM,       "Arm"},
   {MAC_CPU_TYPE_SPARC,     "Sparc"},
   {MAC_CPU_TYPE_POWERPC,   "Power PC 32 bit"},
   {MAC_CPU_TYPE_POWERPC64, "Power PC 64 bit"}
};

// CPU subtype names
SIntTxt MacCPUSubtypeNames[] = {
   {MAC_CPU_SUBTYPE_POWERPC_ALL,  "Power PC All"},
   {MAC_CPU_SUBTYPE_I386_ALL,     "Intel All"}
};

// File type names
SIntTxt MacFileTypeNames[] = {
   {MAC_OBJECT,     "Relocatable object file"},
   {MAC_EXECUTE,    "demand paged executable file"},
   {MAC_FVMLIB,     "fixed VM shared library file"},
   {MAC_CORE,       "core file"},
   {MAC_PRELOAD,    "preloaded executable file"},
   {MAC_DYLIB,      "dynamicly bound shared library file"},
   {MAC_DYLINKER,   "dynamic link editor"},
   {MAC_BUNDLE,     "dynamicly bound bundle file"}
};

// Command type names
SIntTxt MacCommandTypeNames[] = {
   {MAC_LC_SEGMENT,        "Segment"},
   {MAC_LC_SYMTAB,         "Symbol table"},
   {MAC_LC_SYMSEG,         "gdb symbol table info (obsolete)"},
   {MAC_LC_THREAD,         "thread"},
   {MAC_LC_UNIXTHREAD,     "unix thread"},
   {MAC_LC_LOADFVMLIB,     "load a specified fixed VM shared library"},
   {MAC_LC_IDFVMLIB,       "fixed VM shared library identification"},
   {MAC_LC_IDENT,          "object identification info (obsolete)"},
   {MAC_LC_FVMFILE,        "fixed VM file inclusion (internal use)"},
   {MAC_LC_PREPAGE,        "prepage command (internal use)"},
   {MAC_LC_DYSYMTAB,       "dynamic link-edit symbol table info"},
   {MAC_LC_LOAD_DYLIB,     "load a dynamicly linked shared library"},
   {MAC_LC_ID_DYLIB,       "dynamicly linked shared lib identification"},
   {MAC_LC_LOAD_DYLINKER,  "load a dynamic linker"},
   {MAC_LC_ID_DYLINKER,    "dynamic linker identification"},
   {MAC_LC_PREBOUND_DYLIB, "modules prebound for a dynamicly linked shared library"}
};

// Relocation type names
SIntTxt MacRelocationTypeNames[] = {
   {MAC_RELOC_VANILLA,        "Generic"},
   {MAC_RELOC_PAIR,           "Second entry of a pair"},
   {MAC_RELOC_SECTDIFF,       "Section diff"},
   {MAC_RELOC_PB_LA_PTR,      "Prebound lazy ptr"},
   {MAC_RELOC_LOCAL_SECTDIFF, "SectDif local"}
};

// Symbol type names
SIntTxt MacSymbolTypeNames[] = {
   {MAC_N_UNDF,    "Undefined, no section"},
   {MAC_N_ABS,     "Absolute, no section"},
   {MAC_N_SECT,    "Defined"},
   {MAC_N_PBUD,    "Prebound undefined (defined in a dylib)"},
   {MAC_N_INDR,    "Indirect"}
};

// Symbol reference type names
SIntTxt MacSymbolReferenceTypeNames[] = {
   {MAC_REF_FLAG_UNDEFINED_NON_LAZY,         "External non lazy"},
   {MAC_REF_FLAG_UNDEFINED_LAZY,             "External lazy (function call)"},
   {MAC_REF_FLAG_DEFINED,                    "Defined public"},
   {MAC_REF_FLAG_PRIVATE_DEFINED,            "Defined private"},
   {MAC_REF_FLAG_PRIVATE_UNDEFINED_NON_LAZY, "Private undefined non lazy"},
   {MAC_REF_FLAG_PRIVATE_UNDEFINED_LAZY,     "Private undefined lazy"}
};

// Symbol descriptor flag names
SIntTxt MacSymbolDescriptorFlagNames[] = {
   {MAC_REFERENCED_DYNAMICALLY, "Referenced dynamically"},
// {MAC_N_DESC_DISCARDED,       "Discarded"},
   {MAC_N_NO_DEAD_STRIP,        "Don't dead-strip"},
   {MAC_N_WEAK_REF,             "Weak external"},
   {MAC_N_WEAK_DEF,             "Weak public"}
};



// Class CMACHO members:
// Constructor
CMACHO::CMACHO() {
   // Set everything to zero
   memset(this, 0, sizeof(*this));
}

void CMACHO::ParseFile(){
   // Load and parse file buffer
   FileHeader = *(MAC_header*)Buf();   // Copy file header

   // Loop through file commands
   uint32 cmd, cmdsize;
   uint32 currentoffset = sizeof(MAC_header);
   for (uint32 i = 1; i <= FileHeader.ncmds; i++) {
      uint8 * currentp = (uint8*)(Buf() + currentoffset);
      cmd     = ((MAC_load_command*)currentp) -> cmd;
      cmdsize = ((MAC_load_command*)currentp) -> cmdsize;
      // Interpret specific command type
      switch(cmd) {
         case MAC_LC_SEGMENT: {
            MAC_segment_command * sh = (MAC_segment_command*)currentp;
            SegmentOffset = sh->fileoff;              // File offset of segment
            SegmentSize = sh->filesize;               // Size of segment
            NumSections = sh->nsects;                 // Number of sections
            SectionHeaderOffset = currentoffset + sizeof(MAC_segment_command); // File offset of segment headers
            break;}

         case MAC_LC_SYMTAB: {
            MAC_symtab_command * sh = (MAC_symtab_command*)currentp;
            SymTabOffset = sh->symoff;                // File offset of symbol table
            SymTabNumber = sh->nsyms;                 // Number of entries in symbol table
            StringTabOffset = sh->stroff;             // File offset of string table
            StringTabSize = sh->strsize;              // Size of string table
            break;}

         case MAC_LC_DYSYMTAB: {
            MAC_dysymtab_command * sh = (MAC_dysymtab_command*)currentp;
            ilocalsym = sh->ilocalsym;	               // index to local symbols
            nlocalsym = sh->nlocalsym;	               // number of local symbols 
            iextdefsym = sh->iextdefsym;	            // index to externally defined symbols
            nextdefsym = sh->nextdefsym;	            // number of externally defined symbols 
            iundefsym = sh->iundefsym;	               // index to undefined symbols
            nundefsym = sh->nundefsym;	               // number of undefined symbols
            IndirectSymTabOffset = sh->indirectsymoff;// file offset to the indirect symbol table
            IndirectSymTabNumber = sh->nindirectsyms; // number of indirect symbol table entries
            break;}
      }
      currentoffset += cmdsize;
   }
}

// Debug dump
void CMACHO::Dump(int options) {
   uint32 icmd;                        // Command index
   int32  isec1;                       // Section index within segment
   int32  isec2;                       // Section index global
   int32  nsect;                        // Number of sections in segment

   if (options & DUMP_FILEHDR) {
      // File header
      printf("\nDump of Mach-O file %s", FileName);
      printf("\n-----------------------------------------------");
      printf("\nFile size: 0x%X", GetDataSize());
      printf("\nFile header:");
      printf("\n  CPU type: %s, subtype: %s",
         Lookup(MacMachineNames, FileHeader.cputype), 
         Lookup(MacCPUSubtypeNames, FileHeader.cpusubtype));
      
      printf("\n  File type: %s - %s", 
         GetFileFormatName(FileType), Lookup(MacFileTypeNames, FileHeader.filetype));

      printf("\n  Number of load commands: %i, Size of commands: 0x%X, Flags: %X",
         FileHeader.ncmds, FileHeader.sizeofcmds, FileHeader.flags);
   }

   uint32 cmd;                         // Load command
   uint32 cmdsize;                     // Command size
   // Pointer to current position
   uint8 * currentp = (uint8*)(Buf() + sizeof(MAC_header));

   // Loop through file commands
   for (icmd = 1; icmd <= FileHeader.ncmds; icmd++) {
      cmd     = ((MAC_load_command*)currentp) -> cmd;
      cmdsize = ((MAC_load_command*)currentp) -> cmdsize;

      if (options & DUMP_SECTHDR) {
         // Dump command header
         printf("\n\nCommand %i: %s, size: 0x%X", icmd,
         Lookup(MacCommandTypeNames, cmd), cmdsize);

         // Interpret specific command type
         switch(cmd) {
            case MAC_LC_SEGMENT: {
               MAC_segment_command * sh = (MAC_segment_command*)currentp;
               printf("\n  Name: %s, Memory address 0x%X, Memory size 0x%X"
                  "\n  File offset 0x%X, File size 0x%X, Maxprot 0x%X, Initprot 0x%X"
                  "\n  Number of sections %i, Flags 0x%X",
                  sh->segname, sh->vmaddr, sh->vmsize,
                  sh->fileoff, sh->filesize, sh->maxprot, sh->initprot, 
                  sh->nsects, sh->flags);
               break;}

            case MAC_LC_SYMTAB: {
               MAC_symtab_command * sh = (MAC_symtab_command*)currentp;
               printf("\n  Symbol table offset 0x%X, number of symbols %i,"
                  "\n  String table offset 0x%X, String table size 0x%X",
                  sh->symoff, sh->nsyms, sh->stroff, sh->strsize);
               break;}

            case MAC_LC_DYSYMTAB: {
               MAC_dysymtab_command * sh = (MAC_dysymtab_command*)currentp;
               printf("\n  Index to local symbols %i, number of local symbols %i,"
                  "\n  Index to external symbols %i, number of external symbols %i,"
                  "\n  Index to undefined symbols %i, number of undefined symbols %i,"
                  "\n  File offset to TOC 0x%X, number of entries in TOC %i,",
                  sh->ilocalsym, sh->nlocalsym, sh->iextdefsym, sh->nextdefsym, 
                  sh->iundefsym, sh->nundefsym, sh->tocoff, sh->ntoc);
               printf("\n  File offset to module table 0x%X, Number of module table entries %i,"
                  "\n  Offset to referenced symbol table 0x%X, Number of referenced symtab entries %i"
                  "\n  Offset to indirect symbol table 0x%X, Number of indirect symtab entries %i"
                  "\n  Offset to external relocation entries 0x%X, Number of external reloc. entries %i"
                  "\n  Offset to local relocation entries 0x%X, Number of local reloc. entries %i",
                  sh->modtaboff, sh->nmodtab, sh->extrefsymoff, sh->nextrefsyms, 
                  sh->indirectsymoff, sh->nindirectsyms, sh->extreloff, sh->nextrel,
                  sh->locreloff, sh->nlocrel);	
               break;}
         }

      }
      currentp += cmdsize;
   }

   // Dump section headers
   if (options & DUMP_SECTHDR) {
      printf("\n\nSections:");

      // Reset current pointer
      currentp = (uint8*)(Buf() + sizeof(MAC_header));
      isec2 = 0;

      // Loop through load commands
      for (icmd = 1; icmd <= FileHeader.ncmds; icmd++) {
         cmd     = ((MAC_load_command*)currentp) -> cmd;
         cmdsize = ((MAC_load_command*)currentp) -> cmdsize;

         if (cmd == MAC_LC_SEGMENT) {
            // This is a segment command
            // Number of sections in segment
            nsect   = ((MAC_segment_command*)currentp) -> nsects;

            // Find first section header
            MAC_section * sectp = (MAC_section*)(currentp + sizeof(MAC_segment_command));

            // Loop through section headers
            for (isec1 = 1; isec1 <= nsect; isec1++, sectp++) {
               printf("\n\nSection %i: Name: %s, Segment: %s.", 
                  ++isec2, sectp->sectname, sectp->segname);
               printf("\n  Memory address 0x%X, Size 0x%X, File offset 0x%X"
                  "\n  Alignment %i, Reloc. ent. offset 0x%X, Num reloc. %i"
                  "\n  Flags 0x%X, reserved1 0x%X, reserved2 0x%X",
                  sectp->addr, sectp->size, sectp->offset, 1 << sectp->align,
                  sectp->reloff, sectp->nreloc, sectp->flags, 
                  sectp->reserved1, sectp->reserved2);

               if (sectp->nreloc && (options & DUMP_RELTAB)) {
                  // Dump relocations
                  printf("\n  Relocations:");
                  MAC_relocation_info * relp = (MAC_relocation_info*)(Buf() + sectp->reloff);
                  for (uint32 r = 1; r <= sectp->nreloc; r++, relp++) {
                     if (relp->r_address & R_SCATTERED) {
                        // scattered relocation into
                        MAC_scattered_relocation_info * scatp = (MAC_scattered_relocation_info*)relp;
                        if (!(scatp->r_type & MAC_RELOC_PAIR)) {
                           printf ("\n    Offset: 0x%X, Value: 0x%X, Length: %i, Scat. Type: %s",
                              scatp->r_address, scatp->r_value, 1 << scatp->r_length, 
                              Lookup(MacRelocationTypeNames, scatp->r_type));
                           if (scatp->r_address < sectp->size) {
                              printf(", Inline: 0x%X", *(int32*)(Buf()+sectp->offset+scatp->r_address));
                           }
                        }
                        else {
                           // Second entry of a pair
                           printf ("\n     Offset2: 0x%X, Value2: 0x%X, Length2: %i",
                              scatp->r_address, scatp->r_value, 1 << scatp->r_length);
                        }
                        if (scatp->r_pcrel) printf(", PC relative");
                     }
                     else {
                        // non-scattered
                        if (relp->r_extern) printf ("\n    Symbol number %i, ", relp->r_symbolnum);
                        else printf ("\n    Section: %i, ", relp->r_symbolnum);
                        printf ("Offset: 0x%X, ", relp->r_address);
                        if (relp->r_pcrel) printf ("PC relative, ");
                        printf ("\n     Length: %i, Extern: %i, Type: %s",
                           1 << relp->r_length, relp->r_extern,
                           Lookup(MacRelocationTypeNames, relp->r_type));
                        if (relp->r_address < sectp->size) {
                           printf(", Inline: 0x%X", *(int32*)(Buf()+sectp->offset+relp->r_address));
                        }
                     }
                  }
               }
            }
         }
         currentp += cmdsize;
      }
   }

   // pointer to string table
   char * strtab = (char*)(Buf() + StringTabOffset); 
   // pointer to symbol table
   MAC_nlist * symp0 = (MAC_nlist*)(Buf() + SymTabOffset);

   // Dump symbol table
   if (options & DUMP_SYMTAB) {
      printf("\n\nSymbol table:");
      uint32 i;
      MAC_nlist * symp;

      // loop through symbol table
      for (i = 0, symp = symp0; i < SymTabNumber; i++, symp++) {

         // Header for first symbol of each category: (alphabetical within each category)
         if (i == ilocalsym && nlocalsym)   printf("\n\n  Local symbols:");
         if (i == iextdefsym && nextdefsym) printf("\n\n  Public symbols:");
         if (i == iundefsym && nundefsym)   printf("\n\n  External symbols:");

         if (symp->n_strx < StringTabSize && !(symp->n_type & MAC_N_STAB)) {
            printf("\n  %2i %s, Section %i, Value 0x%X\n    ",
               i, strtab + symp->n_strx, symp->n_sect, symp->n_value);
         }
         else {
            printf("\n  String table offset: 0x%X, Section %i, Value 0x%X\n    ",
               symp->n_strx, symp->n_sect, symp->n_value);
         }

         if (symp->n_type & MAC_N_STAB) {
            printf ("Debug symbol, stab = 0x%X, ", symp->n_type);
         }
         else {
            if (symp->n_type & MAC_N_PEXT) printf ("Private external (limited global scope), ");
            if (symp->n_type & MAC_N_EXT ) printf ("External, ");
            printf("%s", Lookup(MacSymbolTypeNames, symp->n_type & MAC_N_TYPE));
         }
         printf("\n    Reference type: %s,  Flags: ",
            Lookup(MacSymbolReferenceTypeNames, symp->n_desc & MAC_REF_TYPE));
         for (uint32 f = MAC_REFERENCED_DYNAMICALLY; f <= MAC_N_WEAK_DEF; f <<= 1) {
            if (symp->n_desc & f) {
               printf("%s, ", Lookup(MacSymbolDescriptorFlagNames, f));
            }
         }
      }
      // Check if indirect symbol table is valid
      if (IndirectSymTabNumber && IndirectSymTabOffset + IndirectSymTabNumber*4 < GetDataSize()) {

         // Write indirect symbol table
         printf("\n\n  Indirect symbols:");

         // loop through indirect symbol table
         uint32 * IndSymip = (uint32*)(Buf() + IndirectSymTabOffset);

         for (i = 0; i < IndirectSymTabNumber; i++, IndSymip++) {

            // Check if index within symbol table
            if (*IndSymip >= SymTabNumber) {
               //err.submit(2016); 
               printf("\n   Unknown(0x%X)", *IndSymip);
               continue;
            }
            // Find name
            uint32 StringIndex = symp0[*IndSymip].n_strx;
            if (StringIndex >= StringTabSize) {
               err.submit(2035); continue;
            }
            // print name
            printf("\n   %s", strtab + StringIndex);
         }
      }
   }

   // Dump string table
   if (options & DUMP_STRINGTB) {
      printf("\n\nString table:");
      uint32 str = 0, istr = 0;
      while (str < StringTabSize) {
         char * p = (char*)(Buf() + StringTabOffset + str);
         printf("\n  %3i: %s", str, p);
         istr++;  str += (uint32)strlen(p) + 1;
      }
   }

}
void CMACHO::PublicNames(CMemoryBuffer * Strings, CSList<SStringEntry> * Index, int m) {
   // Make list of public names
   uint32 i;
   SStringEntry se;                    // Entry in Index

   // Interpret header:
   ParseFile();

   // pointer to string table
   char * strtab = (char*)(Buf() + StringTabOffset); 

   // loop through public symbol table
   MAC_nlist * symp = (MAC_nlist*)(Buf() + SymTabOffset + iextdefsym * sizeof(MAC_nlist));
   for (i = 0; i < nextdefsym; i++, symp++) {
      if (symp->n_strx < StringTabSize && !(symp->n_type & MAC_N_STAB)) {
         // Public symbol found
         se.Member = m;
         // Store name
         se.String = Strings->PushString(strtab + symp->n_strx);         
         // Store name index
         Index->Push(se);
      }
   }
}

// Member functions for class MacSymbolTableBuilder

MacSymbolTableBuilder::MacSymbolTableBuilder() {                       // Constructor
   sorted = 0;
}

void MacSymbolTableBuilder::AddSymbol(int OldIndex, char * name, int type, int Desc, int section, uint32 value) {
   // Add symbol to list
   MacSymbolRecord rec;
   memset(&rec, 0, sizeof(rec));                 // Set to zero
   rec.n_type = (uint8)type;                     // Copy values
   rec.n_sect = (uint8)section;
   rec.n_desc = (int16)Desc;
   rec.n_value = value;
   rec.name = name;                              // Pointer to name
   rec.OldIndex = OldIndex;                      // Remember old index
   Push(&rec, sizeof(rec));                      // Put in memory buffer
   sorted = 0;                                   // Remember not sorted
}

void MacSymbolTableBuilder::SortList() {
   // Sort the list
   MacSymbolRecord * p = (MacSymbolRecord*)Buf();     // Point to list

   // Simple Bubble sort:
   int i, j;  char * s1, * s2;
   MacSymbolRecord temp;
   for (i = 0; i < (int)GetNumEntries(); i++) {
      for (j = 0; j < (int)GetNumEntries() - i - 1; j++) {
         s1 = p[j].name;
         s2 = p[j+1].name;
         if (strcmp(s1, s2) > 0) {
            // Swap records
            temp = p[j];
            p[j] = p[j+1];
            p[j+1] = temp;
         }
      }
   }
   sorted = 1;
}

int MacSymbolTableBuilder::TranslateIndex(int OldIndex) {
   // Translate old index to new index (0-based), after sorting
   // Returns -1 if not found
   if (!sorted) SortList();                           // Make sure list is sorted
   MacSymbolRecord * p = (MacSymbolRecord*)Buf();     // Point to list

   // Search through list for OldIndex
   for (int i = 0; i < (int)GetNumEntries(); i++) {
      if (p[i].OldIndex == OldIndex) {
         // Match found
         return i;
      }
   }
   // Not found
   return -1;
}

void MacSymbolTableBuilder::StoreList(CMemoryBuffer * SymbolTable, CMemoryBuffer * StringTable) {
   // Store sorted list in buffers
   if (!sorted) SortList();                           // Make sure list is sorted
   MacSymbolRecord * p = (MacSymbolRecord*)Buf();     // Point to list

   for (uint32 i = 0; i < GetNumEntries(); i++, p++) {
      p->n_strx = StringTable->PushString(p->name);   // Put name in string table
      SymbolTable->Push(p, sizeof(MAC_nlist));        // Store only the MAC_nlist part of the record in SymbolTable
   }
}

MacSymbolRecord & MacSymbolTableBuilder::operator[] (uint32 i) {
   // Access member
   uint32 Offset = i * sizeof(MacSymbolRecord);
   if (i + sizeof(MacSymbolRecord) > GetDataSize()) {
      err.submit(9003);  Offset = 0;
   }
   return Get<MacSymbolRecord>(Offset);
}

/****** Class CMACUNIV for parsing Macintosh universal binary *************/
CMACUNIV::CMACUNIV() {
   // Default constructor
}

void CMACUNIV::Go(int options) {
   // Apply command options to all components

   // Check file size
   if (GetDataSize() < 28) return;

   // Read number of components
   uint32 NumComponents = EndianChange(Get<MAC_UNIV_FAT_HEADER>(0).num_arch);
   if (NumComponents == 0 || NumComponents > 10) {
      // Number of components too big or too small
      err.submit(2701, NumComponents);
      return;
   }

   uint32 i;                                     // Component number
   uint32 fo;                                    // File offset of component pointer

   // Loop through components
   for (i = 0, fo = sizeof(MAC_UNIV_FAT_HEADER); i < NumComponents; i++, fo += sizeof(MAC_UNIV_FAT_ARCH)) {

      // Get component pointer
      MAC_UNIV_FAT_ARCH & ComponentPointer = Get<MAC_UNIV_FAT_ARCH>(fo);

      // Get offset and size of component
      uint32 ComponentOffset = EndianChange(ComponentPointer.offset);
      uint32 ComponentSize   = EndianChange(ComponentPointer.size);

      // Check within range
      if (ComponentOffset + ComponentSize > GetDataSize()) {
         err.submit(2016);
         return;
      }

      // Put component into buffer
      {
         CConverter ComponentBuffer;
         ComponentBuffer.Push(Buf() + ComponentOffset, ComponentSize);

         // Indicate component
         printf("\n\nComponent file number %i:\n", i + 1);

         // Check type
         uint32 ComponentType = ComponentBuffer.GetFileType();
         if (ComponentType != FILETYPE_MACHO_LE) {
            // Format not supported
            printf("  Format not supported: %s", GetFileFormatName(ComponentType));
         }
         else {
            // Format OK. Handle component
            // Transfer filenames
            ComponentBuffer.FileName = FileName;
            ComponentBuffer.OutputFileName = OutputFileName;
            // Do command
            ComponentBuffer.Go();
            // Is there an output file?
            if (cmd.DumpOptions == 0) {
               // Take over output file and skip remaining components
               *this << ComponentBuffer;
               break;
            }
         }
      }
   }
}
