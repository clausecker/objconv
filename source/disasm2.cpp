/****************************  disasm2.cpp   ********************************
* Author:        Agner Fog
* Date created:  2007-02-25
* Last modified: 2007-06-01
* Project:       objconv
* Module:        disasm2.cpp
* Description:
* Module for disassembler containing file output functions
*
* Changes that relate to assembly language syntax should be done in this file only.
*
* (c) 2007 GNU General Public License www.gnu.org/copyleft/gpl.html
*****************************************************************************/
#include "stdafx.h"

/**********************  Warning and error texts   ***************************
These texts are inserted in disassembled code in case of warnings or errors.

The occurrence of an error makes the disassembler mark the code block between
the nearest known code labels as dubious. This means that the byte sequence
might be data in the code segment or the disassembler might be out of phase
with instruction boundaries. Dubious code will be shown both as code and as
data.

A warning will be shown as 'Note:' before the instruction it applies to.
This might indicate suboptimal coding or a possible cause for concern.

The criteria for distinguishing between warnings and errors is not the 
severity of consequences, but whether the condition is likely to be caused
by common programming errors or by data in the code segment.

Some of the warning messages are quite benign, e.g. an unnecessary prefix.
Other warning messages can have severe consequences, e.g. a function missing
a return statement.

Still other warnings are no case for concern, but a condition requiring 
attention. For example the message: "Multi-byte NOP. Replace with ALIGN",
might actually indicate a well optimized code. But it requires attention
because the assembler cannot re-create the multi-byte NOP if the code
is assembled again. The programmer needs to decide what level of alignment
is optimal and replace the NOP with an align statement.

*****************************************************************************/

// Define error texts. 
SIntTxt AsmErrorTexts[] = {
   {1,         "Instruction longer than 15 bytes"},
   {2,         "Lock prefix not allowed for this opcode"},
   {4,         "Illegal opcode"},
   {8,         "Illegal operands for this opcode"},
   {0x10,      "Instruction extends beyond end of code block"},
   {0x20,      "Prefix after REX prefix not allowed"},
   {0x40,      "This instruction is not allowed in 64 bit mode"},
   {0x80,      "Instruction out of phase with next label"},
   {0x200,     "Attempt to use R13 as base register without displacement"},
   {0x1000,    "Relocation source does not match address or operand field"},
   {0x2000,    "Overlapping relocations"},
   {0x4000,    "This is unlikely to be code"}, // Consecutive bytes of 0 found
   {0x80000,   "Internal error in opcode table in opcodes.cpp"}
};

// Define warning texts. 
SIntTxt AsmWarningTexts[] = {
   {1,          "Immediate operand could be made smaller by sign extension"},
   {2,          "Immediate operand could be made smaller by zero extension"},
   {4,          "Zero displacement could be omitted"},
   {8,          "Displacement could be made smaller by sign extension"},
   {0x10,       "SIB byte unnecessary here"},
   {0x20,       "A shorter instruction exists for register operand"},
   {0x40,       "Length-changing prefix causes delay on Intel processors"},
   {0x80,       "Address size prefix should be avoided"},
   {0x100,      "Same prefix occurs more than once"},
   {0x200,      "Prefix valid but unnecessary"},
   {0x400,      "Prefix has no meaning in this context"},
   {0x800,      "Contradicting prefixes"},
   {0x1000,     "Required prefix missing"},
   {0x2000,     "Address has scale factor but no index register"},
   {0x4000,     "Address is not rip-relative"},
   {0x8000,     "Absolute memory address without relocation"},
   {0x10000,    "Wrong relocation type for this operand"},
   {0x20000,    "Instruction pointer truncated by operand size prefix"},
   {0x40000,    "Stack pointer truncated by address size prefix"},
   {0x80000,    "Jump or call to data segment not allowed"},
   {0x100000,   "Undocumented opcode"},
   {0x200000,   "Unknown opcode reserved for future extensions"},
   {0x400000,   "Memory operand is misaligned"},
   {0x800000,   "XMM operand is misaligned. Must be aligned by 16!"},
   {0x1000000,  "Multi-byte NOP. Replace with ALIGN"},
   {0x2000000,  "Bogus length-changing prefix causes delay on Intel processors here"},
   {0x4000000,  "Non-default size for stack operation"},
   {0x8000000,  "Function does not end with ret or jmp"},
   {0x10000000, "Inaccessible code"}
};

// Indication of relocation types in comments:
SIntTxt RelocationTypeNames[] = {
   {0x001,  "(d)" },                   // Direct address in flat address space
   {0x002,  "(rel)" },                 // Self-relative
   {0x004,  "(imgrel)" },              // Image-relative
   {0x008,  "(segrel)" },              // Segment-relative
   {0x010,  "(refpoint)" },            // Relative to arbitrary point (position-independent code in Mach-O)
   {0x021,  "(d)" },                   // Direct (adjust by image base)
   {0x041,  "(d)" },                   // Direct (make procecure linkage table entry)
   {0x100,  "(seg)" },                 // Segment address or descriptor
   {0x200,  "(sseg)" },                // Segment of symbol
   {0x400,  "(far)" },                 // Far segment:offset address
   {0x1001, "(GOT)" },                 // GOT entry
   {0x1002, "(GOT r)" },               // self-relative to GOT entry
   {0x2002, "(PLT r)" }                // self-relative to PLT entry
};



/**************************  class CDisassembler  *****************************
Most member functions of CDisassembler are defined in disasm1.cpp

Only the functions that produce output are defined here:
******************************************************************************/

void CDisassembler::WriteRegOperand(uint32 Type) {
   // Write register operand from reg bits
   uint32 Num = s.Reg;                           // Register number

   // Write register name
   WriteRegisterName(Num, Type);
}

void CDisassembler::WriteRMOperand(uint32 Type) {
   // Write memory or register operand from mod/rm bits of mod/reg/rm byte 
   // and possibly SIB byte or direct memory operand to OutFile;
   // or register operand from last bits of opcode

   if ((Type & 0xFF) == 0) {
      // No explicit operand
      return;
   }

   uint32 Components = 0;                        // Count number of addends inside []
   int64  Addend = 0;                            // Inline displacement or addend

   // Check if mod/reg/rm byte 
   if ((s.OpcodeDef->InstructionFormat & 0x1F) == 3) {
      // No memory operand. Register operand indicated by last bits of opcode
      uint32 rnum = Get<uint8>(s.OpcodeStart2) & 7;
      // Check REX.B prefix
      if (s.Prefixes[7] & 1) rnum |= 8;           // Add 8 if REX.B prefix
      // Write register name
      WriteRegisterName(rnum, Type);
      return;
   }

   // Check if register or memory
   if (s.Mod == 3) {
      // Register operand
      WriteRegisterName(s.RM, Type);
      return;
   }

   // Find addend, if any
   switch (s.AddressFieldSize) {
   case 1:  // 1 byte displacement
      Addend = Get<int8>(s.AddressField);
      break;
   case 2:  // 2 bytes displacement
      Addend = Get<int16>(s.AddressField);
      break;
   case 4:  // 4 bytes displacement
      Addend = Get<int32>(s.AddressField);
      if ((s.MFlags & 0x100) && !s.AddressRelocation) {
         // rip-relative
         Addend += ImageBase + uint64(SectionAddress + IEnd);
      }
      break;
   case 8:  // 8 bytes address
      Addend = Get<int64>(s.AddressField);
      break;
   }

   // Remove bits that define precision on mmx and xmm operands
   if (Type >= 0x10 && Type < 0x40) Type &= ~7;

   // Make exception for LEA with no type
   if (Opcodei == 0x8D) {
      Type = 0;
   }

   // Write type ptr
   switch (Type & 0xFF) {
   case 1:  // 8 bits
      OutFile.Put("byte ptr ");  break;

   case 2:  // 16 bits
      OutFile.Put("word ptr ");  break;

   case 3:  // 32 bits
      OutFile.Put("dword ptr ");  break;

   case 4:  // 64 bits
      OutFile.Put("qword ptr ");  break;

   case 5:  // 80 bits
      OutFile.Put("tbyte ptr ");  break;

   case 6: case 0x40: case 0x48: case 0:
      // Other size. Write nothing
      break;

   case 7:  // 48 bits
      OutFile.Put("fword ptr ");  break;

   case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: // 64 bits mmx
   case 0x5B: // 2*32 bits float (3DNow)
      OutFile.Put("qword ptr ");  break;

   case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: // 128 bits xmm integer
   case 0x28: // unaligned xmm integer
   case 0x6B: // 128 bits xmm ps
   case 0x6C: // 128 bits xmm pd
      OutFile.Put("xmmword ptr ");  break;

   case 0x43: // 32 bits float (x87)
   case 0x4B: // 32 bits float (SSE2)
      OutFile.Put("dword ptr ");  break;

   case 0x44: // 64 bits float
   case 0x4C: // 64 bits float (SSE2)
      OutFile.Put("qword ptr ");  break;

   case 0x45: // 80 bits float
      OutFile.Put("tbyte ptr ");  break;

   default:   // Should not occur
      OutFile.Put("unknown type ");
      err.submit(3000);
   }

   // Write segment prefix, if any
   if (s.Prefixes[0]) {
      OutFile.Put(RegisterNamesSeg[GetSegmentRegisterFromPrefix()]);
      OutFile.Put(":");
   }
   else if (!s.BaseReg && !s.IndexReg && (!s.AddressRelocation || (s.Warnings & 0x10000))) { 
      // No pointer register and no memory reference or wrong type of memory reference.
      // Write segment register to indicate that we have a memory operand
      OutFile.Put("DS:");
   }

   // Write "[" to enclose memory operands
   OutFile.Put("[");

   // Write relocation target, if any
   if (s.AddressRelocation) {
      // Write cross reference
      WriteRelocationTarget(s.AddressRelocation, 4 | (s.MFlags & 0x100), Addend);
      // Addend has been written, don't write it again
      Addend = 0;
      // Remember that something has been written
      Components++;
   }

   // Check address size for pointer registers
   const char * * PointerRegisterNames;
   switch (s.AddressSize) {
      case 16:
         PointerRegisterNames = RegisterNames16;  break;
      case 32:
         PointerRegisterNames = RegisterNames32;  break;
      case 64:
         PointerRegisterNames = RegisterNames64;  break;
   }

   // Write base register, if any
   if (s.BaseReg) {
      if (Components++) OutFile.Put(" + ");      // Put "+" if anything before
      OutFile.Put(PointerRegisterNames[s.BaseReg - 1]);
   }

   // Write index register, if any
   if (s.IndexReg) {
      if (Components++) OutFile.Put(" + ");      // Put "+" if anything before
      OutFile.Put(PointerRegisterNames[s.IndexReg - 1]);
      // Write scale factor, if any
      if (s.Scale) {
         OutFile.Put("*");
         OutFile.PutDecimal(1 << s.Scale);
      }
   }

   // Write +/- before addend
   if (Components && Addend) {
      // Displacement comes after base/index registers
      if (Addend >= 0 || s.AddressFieldSize == 8) {
         // Positive. Write +
         OutFile.Put(" + ");
      }
      else {
         // Negative. Write -
         OutFile.Put(" - ");
         Addend = -Addend;
      }
   }

   if (Addend || Components == 0) {
      // Write address or addend as hexadecimal
      switch (s.AddressFieldSize) {
      case 1:
         OutFile.PutHex((uint8)Addend, 1);
         break;
      case 2:
         OutFile.PutHex((uint16)Addend, 1);
         break;
      case 4:
         OutFile.PutHex((uint32)Addend, 1);
         break;
      case 8:
         OutFile.PutHex((uint64)Addend, 1);
         break;
      }
   }

   // End with "]"
   OutFile.Put("]");
}

void CDisassembler::WriteRegisterName(uint32 Value, uint32 Type) {
   // Write name of register to OutFile
   Type &= 0xFF;                                         // Remove irrelevant bits

   // Check fixed registers (do not depend on Value)
   switch (Type) {
   case 0xA1:  // al
      Type = 1;  Value = 0;
      break;

   case 0xA2:  // ax
      Type = 2;  Value = 0;
      break;

   case 0xA8:  // ax or eax
      Type = 8;  Value = 0;
      break;

   case 0xA9:  // ax, eax or rax
      Type = 9;  Value = 0;
      break;

   case 0xAF:  // st(0)
      Type = 0x40;  Value = 0;
      break;

   case 0xB2:  // dx
      Type = 2;  Value = 2;
      break;
   }

   // Check if register size depends on mode or prefixes
   switch (Type) {
   case 8:  // 16 or 32 bit general purpose register
      Type = (s.OperandSize == 16) ? 2 : 3;
      break;

   case 9:  // 16, 32 or 64 bit general purpose register
      if (s.OperandSize == 16) Type = 2;
      else if (s.OperandSize == 64) Type = 4;
      else Type = 3;
      break;

   case 0xA:  // 16, 32 or 64 bit general purpose register. Default size = WordSize. REX.W ignored
      if (WordSize == 16) Type = (s.Prefixes[4] == 0x66) ? 3 : 2;
      else if (WordSize == 64) (s.Prefixes[4] == 0x66) ? 2 : 4;
      else (s.Prefixes[4] == 0x66) ? 2 : 3;
      break;

   case 0x30: case 0x31: case 0x32: case 0x33: case 0x34:
      // mmx or xmm operand, depending on 66 prefix
      if (s.Prefixes[5] == 0x66) Type &= ~0x10;  // xmm register
      else Type &= ~0x20;                        // mmx register
      break;
   }

   // Remove bits that define precision on mmx, xmm and st registers
   if (Type >= 0x10 && Type < 0x80) Type &= ~7;

   // Get value within limits
   Value &= 0x0F;

   // Now write register name depending on type
   switch (Type) {
   case 1:  // 8 bit register. Depends on any REX prefix
      OutFile.Put(s.Prefixes[7] ? RegisterNames8x[Value] : RegisterNames8[Value & 7]);
      break;

   case 2:  // 16 bit register
      OutFile.Put(RegisterNames16[Value]);
      break;

   case 3:  // 32 bit register
      OutFile.Put(RegisterNames32[Value]);
      break;

   case 4:  // 64 bit register
      OutFile.Put(RegisterNames64[Value]);
      break;

   case 0x10:  // mmx register (packed integer)
   case 0x58:  // mmx register (3DNow float)
      OutFile.Put("mm");
      OutFile.PutDecimal(Value & 7);
      break;

   case 0x20: case 0x28: // xmm register (packed integer or float)
   case 0x48: case 0x68: // xmm register (scalar float)
      OutFile.Put("xmm");
      OutFile.PutDecimal(Value);
      break;

   case 0x40:  // st register
      OutFile.Put("st(");
      OutFile.PutDecimal(Value & 7);
      OutFile.Put(")");
      break;

   case 0x91:  // Segment register
      OutFile.Put(RegisterNamesSeg[Value & 7]);
      break;

   case 0x92:  // Control register
      OutFile.Put(RegisterNamesCR[Value]);
      break;

   case 0x93:  // Debug register
      OutFile.Put("dr");
      OutFile.PutDecimal(Value);
      break;

   case 0x94:  // Test register (obsolete)
      OutFile.Put("tr");
      OutFile.PutDecimal(Value);
      break;

   case 0xB2:  // 1
      OutFile.Put("1");
      break;

   default:    // Unexpected
      OutFile.Put("UNKNOWN REGISTER TYPE ");
      OutFile.PutDecimal(Value);
      break;
   }
}


void CDisassembler::WriteImmediateOperand(uint32 Type) {
   // Write immediate operand or direct jump/call address
   int    Components = 0;                        // Number of components in immediate operand       
   int64  Value = 0;                             // Value of immediate operand
   uint32 Value2;                                // Value of second immediate operand

   // Check if far
   if ((Type & 0xFE) == 0x84) {
      // Write far ptr
      OutFile.Put("far ptr ");
   }

   // Check if type override needed
   if ((s.OpcodeDef->AllowedPrefixes & 2) && s.Prefixes[4] == 0x66
   && (Opcodei == 0x68 || Opcodei == 0x6A)) {
      // Push immediate with non-default operand size needs type override
      OutFile.Put((s.OperandSize == 16) ? "word ptr " : "dword ptr ");
   }

   // Get inline value
   switch (s.ImmediateFieldSize) {
      case 1:  // 8 bits
         Value = Get<int8>(s.ImmediateField);  break;

      case 2:  // 16 bits
         Value = Get<int16>(s.ImmediateField);  break;

      case 6:  // 48 bits
         Value  = Get<int32>(s.ImmediateField);  
         Value += (uint64)Get<uint16>(s.ImmediateField + 4) << 32;  
         break;

      case 4:  // 32 bits
         Value = Get<int32>(s.ImmediateField);  break;

      case 8:  // 64 bits
         Value = Get<int64>(s.ImmediateField);  break;

      case 3:  // 16 + 8 bits ("enter" instruction)
         Value  = Get<int16>(s.ImmediateField);
         Value2 = Get<uint8>(s.ImmediateField + 2);
         break;

      default:  // Other sizes should not occur
         err.submit(3000);  Value = -1;
   }

   // Check if relocation
   if (s.ImmediateRelocation) {
      // Write relocation target name
      uint32 Context = 2;
      if ((Type & 0xFC) == 0x80) Context = 8;     // Near jump/call destination
      if ((Type & 0xFC) == 0x84) Context = 0x10;  // Far jump/call destination

      // Write cross reference
      WriteRelocationTarget(s.ImmediateRelocation, Context, Value);

      // Remember that Value has been written
      Value = 0;
      Components++;
   }
   // Check if AAM or AAD
   if (Value == 10 && (Opcodei == 0xD4 || Opcodei == 0xD5)) {
      // Don't write operand for AAM or AAD if = 10
      return;
   }

   // Write as decimal or hexadecimal:
   int WriteAsHex = (Type & 0x4000) || (Type & 0xF0) == 0x80 || (s.ImmediateFieldSize == 8 && Value != (int32)Value);

   if ((Type & 0xFC) == 0x80 && !s.ImmediateRelocation) {
      // Self-relative jump or call without relocation. Adjust immediate value
      Value += IEnd;                             // Get absolute address of target

      // Look for symbol at target address
      uint32 ISymbol = Symbols.FindByAddress(Section, (uint32)Value);
      if (ISymbol && (Symbols[ISymbol].Name || CodeMode == 1)) {
         // Symbol found. Write its name
         OutFile.Put(Symbols.GetName(ISymbol));
         // No offset to write
         return;
      }
      // Target address has no name
      Type |= 0x4000;                            // Write target as hexadecimal
   }

   if (Components) {
      // There was a relocated name
      if (Value) {
         // Addend to relocation is not zero
         if (Value > 0 || WriteAsHex) {
            OutFile.Put(" + ");               // Put "+" between name and addend
         }
         else {
            OutFile.Put(" - ");               // Put "-" between name and addend
            Value = - Value;                  // Change sign to avoid another "-"
         }
      }
      else {
         // No addend to relocated name
         return;
      }
   }
   // Write value
   if (WriteAsHex) {
      // Write as hexadecimal
      uint8 OSize = s.OperandSize;
      // Check if destination is 8 bit operand
      if ((s.Operands[0] & 0xFF) == 1 || (s.Operands[0] & 0xFF) == 0xA1) OSize = 8;
      // Write with hexadecimal number appropriate size
      switch (OSize) {
         case 8:  // 8 bits
            OutFile.PutHex((uint8)Value, 1);  break;
         case 16:  // 16 bits
            if ((Type & 0xFC) == 0x84) {
               // Segment of far call
               OutFile.PutHex((uint16)(Value >> 16), 1);
               OutFile.Put(':');
            }
            OutFile.PutHex((uint16)Value, 1);  break;
         case 32:  // 32 bits
         default:  // Should not occur
            if ((Type & 0xFC) == 0x84) {
               // Segment of far call
               OutFile.PutHex((uint16)(Value >> 32), 1);
               OutFile.Put(':');
            }
            OutFile.PutHex((uint32)Value, 1);  break;
         case 64:  // 64 bits
            OutFile.PutHex((uint64)Value, 1);  break;
      }
   }
   else {
      // Write as signed decimal
      OutFile.PutDecimal((int32)Value, 1);    // Write value. Signed decimal
   }
   // Check for second immediate operand
   if (s.ImmediateFieldSize == 3) {
      OutFile.Put(", ");
      if (WriteAsHex) {
         OutFile.PutHex((uint8)Value2, 1);
      }
      else {
         OutFile.PutDecimal((uint32)Value2, 0);
      }
   }
}

void CDisassembler::WriteOtherOperand(uint32 Type) {
   // Write other type of operand
   const char * * OpRegisterNames;               // Pointer to list of register names
   uint32 RegI = 0;                              // Index into list of register names

   switch (Type & 0x8FF) {
   case 0xA1:  // AL
      OpRegisterNames = RegisterNames8;
      break;
   case 0xA2:  // AX
      OpRegisterNames = RegisterNames16;
      break;
   case 0xA8:  // AX or EAX
      OpRegisterNames = (s.OperandSize == 16) ? RegisterNames16 : RegisterNames32;
      break;
   case 0xA9:  // AX, EAX or RAX
      OpRegisterNames = (s.OperandSize == 16) ? RegisterNames16 : RegisterNames32;
      if (s.OperandSize == 64) OpRegisterNames = RegisterNames64;
      break;
   case 0xB2:  // DX
      OpRegisterNames = RegisterNames16;
      RegI = 2;
      break;
   case 0x800: // CL
      OpRegisterNames = RegisterNames8;
      RegI = 1;
      break;
   case 0xAF:  // ST(0)
      OutFile.Put("st(0)");
      return;
   case 0xB1:  // 1
      OutFile.Put("1");
      return;
   default:
      OutFile.Put("unknown operand");
      err.submit(3000);
      return;
   }
   // Write register name
   OutFile.Put(OpRegisterNames[RegI]);
}


void CDisassembler::WriteErrorsAndWarnings() {
   // Write errors, warnings and comments, if any
   uint32 n;                                     // Error bit
   if (s.Errors) {
      // There are errors
      // Loop through all bits in s.Errors
      for (n = 1; n; n <<= 1) {
         if (s.Errors & n) {
            if (OutFile.GetColumn()) OutFile.NewLine(); 
            OutFile.Put(CommentSeparator);       // Write "\n; "
            OutFile.Put("Error: ");              // Write "Error: "
            OutFile.Put(Lookup(AsmErrorTexts,n));// Write error text
            OutFile.NewLine(); 
         }
      }
   }

   if (s.Warnings) {
      // There are warnings
      // Loop through all bits in s.Warnings
      for (n = 1; n; n <<= 1) {
         if (s.Warnings & n) {
            if (OutFile.GetColumn()) OutFile.NewLine(); 
            OutFile.Put(CommentSeparator);       // Write "; "
            OutFile.Put("Note: ");               // Write "Note: "
            OutFile.Put(Lookup(AsmWarningTexts, n));// Write warning text
            OutFile.NewLine(); 
         }
      }
   }
   else if (s.Prefixes[0] && (s.OpcodeDef->AllowedPrefixes & 8)) {
      // Branch hint prefix. Write comment
      OutFile.Put(CommentSeparator);             // Write "; "
      switch (s.Prefixes[0]) {
      case 0x2E:
         OutFile.Put("Branch hint prefix for Pentium 4: Predict no jump");   
         break;
      case 0x3E:
         OutFile.Put("Branch hint prefix for Pentium 4: Predict jump");   
         break;
      case 0x64:
         OutFile.Put("Branch hint prefix for Pentium 4: Predict alternate");   
         break;
      default:
         OutFile.Put("Note: Unrecognized branch hint prefix");   
      }
      OutFile.NewLine(); 
   }
}

void CDisassembler::WriteSymbolName(uint32 symi) {
   // Write symbol name. symi = new symbol index
   OutFile.Put(Symbols.GetName(symi));
}

void CDisassembler::WriteSectionName(int32 SegIndex) {
   // Write name of section, segment or group from section index
   const char * Name = 0;
   // Check for special index values
   switch (SegIndex) {
   case ASM_SEGMENT_UNKNOWN:   // Unknown segment. Typical for external symbols
      Name = "Unknown";  break;
   case ASM_SEGMENT_ABSOLUTE:  // No segment. Used for absolute symbols
      Name = "Absolute";  break;
   case ASM_SEGMENT_FLAT:      // Flat segment group
      Name = "flat";  break;
   case ASM_SEGMENT_NOTHING:   // No segment
      Name = "Nothing";  break;
   case ASM_SEGMENT_ERROR:     // Segment register assumed to error
      Name = "Error";  break;
   case ASM_SEGMENT_IMGREL:    // Segment unknown. Offset relative to image base or file base
      Name = "ImageBased";  break;
   default:                    // > 0 means normal segment index
      if ((uint32)SegIndex >= Sections.GetNumEntries()) {
         // Out of range
         Name = "IndexOutOfRange";
      }
      else {
         // Get index into NameBuffer
         uint32 NameIndex = Sections[SegIndex].Name;
         // Check if valid
         if (NameIndex == 0 || NameIndex >= NameBuffer.GetDataSize()) {
            Name = "ErrorNameMissing";
         }
         else {
            // Normal valid name of segment, section or group
            Name = NameBuffer.Buf() + NameIndex;
         }
      }
      break;
   }
   // Write name
   OutFile.Put(Name);
}

void CDisassembler::WriteDataItems() {
   // Write data items to output file

   int LineFinished;                             // 1 = new line needed, 2 = DB needed, 4 = comment has been written
   uint32 Pos = IBegin;                          // Current position
   uint32 LinePos = IBegin;                      // Position for beginning of output line
   uint32 BytesPerLine;                          // Number of bytes to write per line
   uint32 LineEnd;                               // Data position for end of line
   uint32 DataEnd;                               // End of data
   uint32 ElementSize, OldElementSize;           // Size of each data element
   uint32 RelOffset;                             // Offset of relocation
   int64  Value;                                 // Inline value or addend

   SARelocation Rel;                             // Dummy relocation record

   // Check if size is valid
   if (DataSize == 0) DataSize = 1;
   if (DataSize > 16) DataSize = 16;  

   // Expected end position
   if (CodeMode & 3) {
      // Writing data for dubious code. Make same length as code instruction
      DataEnd = IEnd;
   }
   else {
      // Regular data. End at next label
      DataEnd = LabelEnd;
      if (DataEnd > FunctionEnd) DataEnd = FunctionEnd;
      if (DataEnd <= Pos) DataEnd = Pos + DataSize;
      if (DataEnd > Sections[Section].InitSize && Pos < Sections[Section].InitSize) {
         DataEnd = Sections[Section].InitSize;
      }
   }

   // Size of each data element
   ElementSize = DataSize;

   // Check if packed type
   if (DataType & 0x70) {
      // This is a packed vector type. Get element size
      ElementSize = GetDataElementSize(DataType);
   }

   // Avoid sizes that are not powers of 2
   if (ElementSize == 6 || ElementSize == 10) ElementSize = 2;

   // Set maximum element size to 8
   if (ElementSize > 8)  ElementSize = 8;

   // Set minimum element size to 1
   if (ElementSize < 1)  ElementSize = 1;

   // Set number of bytes per line
   BytesPerLine = (DataSize == 10) ? 10 : 8;

   if (!(CodeMode & 3)) {
      // Begin new line for each data item (except in code segment)
      OutFile.NewLine();
   }
   LineFinished = 1;

   // Check if alignment required
   if (DataSize == 16 && (DataType & 0x20) && (DataType & 0xFF) != 0x28 
   && !(FlagPrevious & 0x100) && !(IBegin & 0xF)) {
      // Write align directive
      OutFile.Put("ALIGN");  OutFile.Tabulate(AsmTab1);  OutFile.Put("16");       
      OutFile.NewLine();
      // Remember that data is aligned
      FlagPrevious |= 0x100;
   }

   // Get symbol name for label
   uint32 sym;                                   // Current symbol index
   uint32 sym1, sym2 = 0;                        // First and last symbol at current address

   sym1 = Symbols.FindByAddress(Section, Pos, &sym2);

   // Loop for one or more symbols at this address
   for (sym = sym1; sym <= sym2; sym++) {

      if (sym && Symbols[sym].Scope && !(Symbols[sym].Scope & 0x100) && !(Symbols[sym].Type & 0x80000000)) {

         // Write symbol label
         OutFile.Put(Symbols.GetName(sym));
         // At least one space
         OutFile.Put(" ");
         // Tabulate
         OutFile.Tabulate(AsmTab1);
         // Get size
         uint32 Size1 = Symbols[sym].Size;
         if (Size1 == 0) Size1 = DataSize;

         if (ElementSize != DataSize || Size1 != DataSize || OutFile.GetColumn() > AsmTab1 
         || sym < sym2 || (Sections[Section].Type & 0xFF) == 3
         || (Symbols[sym].Type+1 & 0xFE) == 0x0C) {

            // Write label and type on seperate line
            OutFile.Put("label ");

            // Write type
            switch(Size1) {
            case 1: default:
               OutFile.Put("byte");  break;
            case 2:
               OutFile.Put("word");  break;
            case 4:
               OutFile.Put("dword");  break;
            case 6:
               OutFile.Put("fword");  break;
            case 8:
               OutFile.Put("qword");  break;
            case 10:
               OutFile.Put("tbyte");  break;
            case 16:
               OutFile.Put("xmmword");  break;
            }
            // Check if jump table or call table
            if ((Symbols[sym].Type+1 & 0xFE) == 0x0C) {
               OutFile.Tabulate(AsmTab3);
               OutFile.Put(CommentSeparator);
               if (Symbols[sym].DLLName) {
                  // DLL import
                  OutFile.Put("import from ");
                  OutFile.Put(Symbols.GetDLLName(sym));
               }
               else if (Symbols[sym].Type & 1) {
                  OutFile.Put("switch/case jump table");
               }
               else {
                  OutFile.Put("virtual table or function pointer");
               }
            }

            // New line
            OutFile.NewLine();
            // Avoid comment
            LineFinished = 6;
         }
         else {
            // Write DB directive but no linefeed
            LineFinished = 2;
         }
      }
   }

   if ((Sections[Section].Type & 0xFF) == 3 || Pos >= Sections[Section].InitSize) {
      // This is an unitialized data (BSS) section
      // Data repeat count
      uint32 DataCount = (DataEnd - Pos) / ElementSize;
      if (DataCount) {
         // Align
         OutFile.Tabulate(AsmTab1);

         // Write data definition directive for appropriate size
         switch (ElementSize) {
         case 1:
            OutFile.Put("db ");  break;
         case 2:
            OutFile.Put("dw ");  break;
         case 4:
            OutFile.Put("dd ");  break;
         case 6:
            OutFile.Put("df ");  break;
         case 8:
            OutFile.Put("dq ");  break;
         case 10:
            OutFile.Put("dt ");  break;
         }
         // Align
         OutFile.Tabulate(AsmTab2);
         if (DataCount > 1) {
            // Write duplication operator
            OutFile.PutDecimal(DataCount);
            OutFile.Put(" dup (?)");
         }
         else {
            // DataCount == 1
            OutFile.Put("?");
         }
         // Write comment
         WriteDataComment(ElementSize, Pos, Pos, 0);
         OutFile.NewLine();
      }
      // Update data position
      Pos += DataCount * ElementSize;

      if (Pos < DataEnd) {
         // Some odd data remain. Write as bytes
         DataCount = DataEnd - Pos;
         ElementSize = 1;
         OutFile.Tabulate(AsmTab1);
         OutFile.Put("db ");
         OutFile.Tabulate(AsmTab2);
         if (DataCount > 1) {
            // Write duplication operator
            OutFile.PutDecimal(DataCount);
            OutFile.Put(" dup (?)");
         }
         else {
            // DataCount == 1
            OutFile.Put("?");
         }
         OutFile.NewLine();
         Pos = DataEnd;
      }
   }
   else {
      // Not a BSS section

      // Loop for one or more elements
      LinePos = Pos;
      while (Pos < DataEnd) {

         // Find end of line position
         LineEnd = LinePos + BytesPerLine;

         // Remember element size
         OldElementSize = ElementSize;

         // Check if relocation
         Rel.Section = Section;
         Rel.Offset  = Pos;
         uint32 irel = Relocations.FindFirst(Rel);
         if (irel >= Relocations.GetNumEntries() || Relocations[irel].Section != Section) {
            // No relevant relocation
            irel = 0;
         }
         if (irel) {
            // A relocation is found
            // Check relocation source
            RelOffset = Relocations[irel].Offset;
            if (RelOffset == Pos) {
               // Relocation source is here
               // Make sure the size fits and begin new line
               ElementSize = Relocations[irel].Size;  BytesPerLine = 8;
               if (ElementSize < 1) ElementSize = WordSize / 8;
               if (ElementSize < 1) ElementSize = 4;
               LineEnd = Pos + ElementSize;
               LineFinished = 1;
            }
            else if (RelOffset < Pos + ElementSize) {
               // Relocation source begins before end of element with current ElementSize
               // Change ElementSize to make sure a new element begins at relocation source
               ElementSize = 1;  BytesPerLine = 8;
               LineEnd = RelOffset;
               LineFinished = 1;
               irel = 0;
            }
            else {
               // Relocation is after this element
               irel = 0;
            }
            // Check for overlapping relocations
            if (irel && irel+1 < Relocations.GetNumEntries() 
               && Relocations[irel+1].Section == Section
               && Relocations[irel+1].Offset < RelOffset + ElementSize) {
                  // Overlapping relocations
                  s.Errors |= 0x2000;
                  WriteErrorsAndWarnings();
                  LineEnd = Relocations[irel+1].Offset;
                  LineFinished = 1;
               }
               // Drop alignment
               FlagPrevious &= ~0x100;
         }
         if (irel == 0) {
            // No relocation here
            if (ElementSize == 6 || ElementSize >= 10) {
               // Avoid odd element size
               ElementSize = 1;  BytesPerLine = 8;
               LineEnd = DataEnd;
               LineFinished = 1;
               FlagPrevious &= ~0x100;                 // Drop alignment
            }
            // Check if DataEnd would be exceeded
            if (Pos + ElementSize > DataEnd) {
               // Make sure we end at DataEnd unless there is a relocation source here
               ElementSize = 1;  BytesPerLine = 8;
               LineEnd = DataEnd;
               LineFinished = 1;
               FlagPrevious &= ~0x100;                 // Drop alignment
            }
         }

         // Check if new line needed
         if (LineFinished & 3) {

            // Begin new line
            if ((LineFinished & 1) && OutFile.GetColumn()) {
               if (Pos == LinePos) {
                  // Name has been written. Need to write 'LABEL' before new line
                  OutFile.Put("label ");
                  switch(ElementSize) {
                  case 1: default:
                     OutFile.Put("byte");  break;
                  case 2:
                     OutFile.Put("word");  break;
                  case 4:
                     OutFile.Put("dword");  break;
                  case 6:
                     OutFile.Put("fword");  break;
                  case 8:
                     OutFile.Put("qword");  break;
                  case 10:
                     OutFile.Put("tbyte");  break;
                  case 16:
                     OutFile.Put("xmmword");  break;
                  }
               }
               // Write comment if data segment
               if (!(CodeMode & 3) && Pos > LinePos) {
                  WriteDataComment(OldElementSize, LinePos, Pos, irel);
               }
               OutFile.NewLine(); 
            }         
            LineFinished = 0;
            LinePos = Pos;
            if (LineEnd <= LinePos) {
               LineEnd = LinePos + BytesPerLine;
            }

            // Tabulate
            OutFile.Tabulate(AsmTab1);

            // Write data definition directive for appropriate size
            switch (ElementSize) {
            case 1:
               OutFile.Put("db ");  break;
            case 2:
               OutFile.Put("dw ");  break;
            case 4:
               OutFile.Put("dd ");  break;
            case 6:
               OutFile.Put("df ");  break;
            case 8:
               OutFile.Put("dq ");  break;
            case 10:
               OutFile.Put("dt ");  break;
            case 16:
               OutFile.Put("xmmword ");  break;
            }
         }
         else {
            // Not the first element, write comma
            OutFile.Put(", ");
         }

         // Get inline value
         switch (ElementSize) {
         case 1:
            Value = Get<int8>(Pos);
            break;
         case 2:
            Value = Get<int16>(Pos);
            break;
         case 4:
            Value = Get<int32>(Pos);
            break;
         case 6:
            Value = Get<uint32>(Pos) + ((uint64)Get<uint16>(Pos+4) << 32);
            break;
         case 8:
            Value = Get<int64>(Pos);
            break;
         case 10: 
            Value = Get<int64>(Pos); //?
            break;
         }

         if (irel) {
            // There is a relocation here. Write the name etc.
            WriteRelocationTarget(irel, 1, Value);
         }
         else {
            // Write value
            switch (ElementSize) {
            case 1:
               OutFile.PutHex((uint8)Value, 1);  
               break;
            case 2:
               OutFile.PutHex((uint16)Value, 1);  
               break;
            case 4:
               OutFile.PutHex((uint32)Value, 1);  
               break;
            case 6:
               OutFile.PutHex((uint16)(Value >> 32), 1);  
               OutFile.Put(":");
               OutFile.PutHex((uint32)Value, 1);  
               break;
            case 8:
               OutFile.PutHex((uint64)Value, 1);  
               break;
            case 10:
               OutFile.Put("??");
               break;
            }
         }

         // Increment position
         Pos += ElementSize;

         // Check if end of line
         if (Pos >= LineEnd || Pos >= DataEnd) LineFinished = 1;

         if (LineFinished) {
            // End of line
            if (!(CodeMode & 3)) {
               // Write comment
               WriteDataComment(ElementSize, LinePos, Pos, irel);
            }
            OutFile.NewLine();
         }
      }
   }

   // Indicate end
   if (IEnd < Pos) IEnd = Pos;
   if (IEnd > LabelEnd) IEnd = LabelEnd;
   if (IEnd > FunctionEnd && FunctionEnd) IEnd = FunctionEnd;

   // Reset FlagPrevious if not aligned
   if (DataSize != 16 || (DataType & 0xFF) == 0x28) FlagPrevious = 0;
}

void CDisassembler::WriteDataComment(uint32 ElementSize, uint32 LinePos, uint32 Pos, uint32 irel) {
   // Write comment after data item
   uint32 pos1;                            // Position of data for comment
   char TextBuffer[64];                    // Buffer for writing floating point number

   OutFile.Tabulate(AsmTab3);              // Tabulate to comment field
   OutFile.Put(CommentSeparator);          // Start comment

   // Write address
   if (SectionEnd + SectionAddress + (uint32)ImageBase > 0xFFFF) {
      // Write 32 bit address
      OutFile.PutHex(LinePos + SectionAddress + (uint32)ImageBase);
   }
   else {
      // Write 16 bit address
      OutFile.PutHex((uint16)(LinePos + SectionAddress));
   }

   if ((Sections[Section].Type & 0xFF) == 3 || Pos > Sections[Section].InitSize) {
      // Unitialized data. Write no data
      return;
   }

   // Space after address
   OutFile.Put(" _ ");

   // Comment type depends on ElementSize and DataType
   switch (ElementSize) {
   case 1:
      // Bytes. Write ASCII characters
      for (pos1 = LinePos; pos1 < Pos; pos1++) {
         // Get character
         int8 c = Get<int8>(pos1);
         // Avoid non-printable characters
         if (c < ' ' || c == 0x7F) c = '.';
         // Print ASCII character
         OutFile.Put(c);
      }
      break;
   case 2:
      // Words. Write as decimal
      for (pos1 = LinePos; pos1 < Pos; pos1 += 2) {
         OutFile.PutDecimal(Get<int16>(pos1), 1);
         OutFile.Put(' ');
      }
      break;
   case 4:
      // Dwords      
      for (pos1 = LinePos; pos1 < Pos; pos1 += 4) {
         if ((DataType & 0x47) == 0x43) {
            // Write as float
            sprintf(TextBuffer, "%.8G", Get<float>(pos1));
            OutFile.Put(TextBuffer);
            // Make sure the number has a . or E to indicate a floating point number
            if (!strchr(TextBuffer,'.') && !strchr(TextBuffer,'E')) OutFile.Put(".0");
         }
         else if (((DataType + 1) & 0xFF) == 0x0C) {
            // jump/call address or offset. Write as hexadecimal
            OutFile.PutHex(Get<uint32>(pos1));
         }
         else {
            // Other. Write as decimal
            OutFile.PutDecimal(Get<int32>(pos1), 1);
         }
         OutFile.Put(' ');
      }
      break;                 
   case 8:
      // Qwords
      for (pos1 = LinePos; pos1 < Pos; pos1 += 8) {
         if ((DataType & 0x47) == 0x44) {
            // Write as double
            sprintf(TextBuffer, "%.16G", Get<double>(pos1));
            OutFile.Put(TextBuffer);
            // Make sure the number has a . or E to indicate a floating point number
            if (!strchr(TextBuffer,'.') && !strchr(TextBuffer,'E')) OutFile.Put(".0");
         }
         else {
            // Write as hexadecimal
            OutFile.PutHex(Get<uint64>(pos1), 1);
         }
         OutFile.Put(' ');
      }
      break;
   case 10:
      // tbyte. Many compilers do not support long doubles in sprintf. Write as bytes
      for (pos1 = LinePos; pos1 < Pos; pos1++) {
         OutFile.PutHex(Get<uint8>(pos1), 1);
      }
      break;
   }

   if (irel && irel < Relocations.GetNumEntries() && Relocations[irel].Offset == LinePos) {
      // Value is relocated
      uint32 RelType = Relocations[irel].Type;
      if (RelType) {
         OutFile.Put(Lookup(RelocationTypeNames, RelType));
      }
   }
}

void CDisassembler::WriteRelocationTarget(uint32 irel, uint32 Context, int64 Addend) {
   // Write cross reference, including addend, but not including segment override and []
   // irel = index into Relocations
   // Context:
   // 1      = Data definition
   // 2      = Immediate data field in instruction
   // 4      = Data address in instruction
   // 8      = Near jump/call destination
   // 0x10   = Far  jump/call destination
   // 0x100  = Self-relative address expected
   // Addend:  inline addend
   // Implicit parameters:
   // IBegin:  value of '$' operator
   // IEnd:    reference point for self-relative addressing
   // BaseReg, IndexReg

   uint32 RefFrame;                    // Target segment

   // Get relocation type
   uint32 RelType = Relocations[irel].Type;

   if (RelType & 0x60) {
      // Inline addend is already relocated. 
      // Ignore addend and treat as direct relocation
      RelType = 1;
      Addend = 0;
   }

   // Get relocation size
   uint32 RelSize = Relocations[irel].Size;

   // Get relocation addend
   Addend += Relocations[irel].Addend;

   // Get relocation target
   uint32 Target = Relocations[irel].TargetOldIndex;

   // Is offset operand needed?
   if (((RelType & 0xB) && (Context & 3)) || ((RelType & 8) && (Context & 0x108))) {
      // offset operator needed to convert memory operand to immediate address
      OutFile.Put("offset ");
   }

   // Is seg operand needed?
   if (RelType & 0x200) {
      // seg operator needed to convert memory operand to its segment
      OutFile.Put("seg ");
   }

   // Is explicit segment or frame needed?
   if ((RelType & 0x408) && (Context & 0x11B)) {
      // Write name of segment/group frame
      RefFrame = Relocations[irel].RefOldIndex;
      if (!RefFrame) {
         // No frame. Use segment of symbol
         RefFrame = Symbols[Symbols.Old2NewIndex(Target)].Section;
      }
      if (RefFrame && RefFrame < Sections.GetNumEntries()) {
         // Write segment or group name
         const char * SecName = NameBuffer.Buf()+Sections[RefFrame].Name;
         OutFile.Put(SecName);
         OutFile.Put(":");
      }
   }

   // Is imagerel operator needed?
   if (RelType & 4) {
      // imagerel operator needed to get image-relative address
      OutFile.Put("imagerel(");
   }

   // Adjust addend
   // Adjust offset if self-relative relocation expected and found
   if ((RelType & 2) && (Context & 0x108)) {
      // Self-relative relocation expected and found
      // Adjust by size of address field and immediate field
      Addend += IEnd - Relocations[irel].Offset;
   }
   // Subtract self-reference if unexpected self-relative relocation
   if ((RelType & 2) && !(Context & 0x108)) {
      // Self-relative relocation found but not expected
      // Fix difference between '$' and reference point
      Addend += IBegin - Relocations[irel].Offset;
   }
   // Add self-reference if self-relative relocation expected but not found
   if (!(RelType & 2) && (Context & 0x108)) {
      // Self-relative relocation expected but not found
      // Fix difference between '$' and reference point
      Addend += IEnd - IBegin;
   }

   if (RelType & 0x100) {
      // Target is a segment
      RefFrame = Symbols[Symbols.Old2NewIndex(Target)].Section;
      if (RefFrame && RefFrame < Sections.GetNumEntries()) {
         const char * SecName = NameBuffer.Buf()+Sections[RefFrame].Name;
         OutFile.Put(SecName);
      }
      else {
         OutFile.Put("undefined segment");
      }
   }
   else {
      // Target is a symbol

      // Find target symbol
      uint32 TargetSym = Symbols.Old2NewIndex(Target);

      // Check if Target is appropriate
      if ((Symbols[TargetSym].Type & 0x80000000) || (int32)Addend) {
         // Symbol is a start-of-section entry in symbol table, or has an addend
         // Look for a more appropriate symbol
         uint32 sym, sym1, sym2 = 0;
         sym1 = Symbols.FindByAddress(Symbols[TargetSym].Section, Symbols[TargetSym].Offset + (int32)Addend, &sym2);
         for (sym = sym1; sym && sym <= sym2; sym++) {
            if (Symbols[sym].Scope && !(Symbols[sym].Type & 0x80000000)) {
               // Found a better symbol name for target address
               TargetSym = sym;
               Addend = 0;
            }
         }
      }
      // Write name of target symbol
      if (TargetSym && Symbols[TargetSym].Name) {
         // Target name valid. Write it
         OutFile.Put(Symbols.GetName(TargetSym));
      }
      else {
         // Invalid
         OutFile.Put("Unnamed_Symbol");
      }
   }

   // End parenthesis if we started one
   if (RelType & 4) {
      OutFile.Put(")");
   }

   // Subtract reference point, if any
   if (RelType & 0x10) {
      OutFile.Put(" - ");
      // Write name of segment/group frame
      uint32 RefPoint = Relocations[irel].RefOldIndex;
      if (RefPoint) {
         // Reference point name valid
         OutFile.Put(Symbols.GetNameO(RefPoint));
      }
      else {
         OutFile.Put("Reference_Point_Missing");
      }
   }

   // Subtract self-reference if unexpected self-relative relocation
   if ((RelType & 2) && !(Context & 0x108)) {
      // Self-relative relocation found but not expected
      OutFile.Put(" - $");
   }

   // Add self-reference if self-relative relocation expected but not found
   if (!(RelType & 2) && (Context & 0x108)) {
      // Self-relative relocation expected but not found
      OutFile.Put(" + $");
   }

   // Write addend, if not zero
   if (Addend) {
      if (Addend < 0) {
         // Negative, write "-"
         OutFile.Put(" - ");
         Addend = -Addend;
      }
      else {
         // Positive, write "+"
         OutFile.Put(" + ");
      }

      // Write value as hexadecimal
      switch (RelSize) {
      case 1:
         OutFile.PutHex((uint8)Addend, 1);
         break;
      case 2:
         OutFile.PutHex((uint16)Addend, 1);
         break;
      case 4:
         OutFile.PutHex((uint32)Addend, 1);
         break;
      case 6:
         OutFile.PutHex((uint16)(Addend >> 32), 1);
         OutFile.Put(":");
         OutFile.PutHex((uint32)Addend, 1);
         break;
      case 8:
         OutFile.PutHex((uint64)Addend, 1);
         break;
      default:
         OutFile.Put("??"); // Should not occur
         break;
      }
   }
}

int CDisassembler::WriteFillers() {
   // Check if code is a series of NOPs or other fillers. 
   // If so then write it as filler and return 1.
   // If not, then return 0.

   // Check if code is filler
   if (!(OpcodeOptions & 0x40)) {
      // This instruction can not be used as filler
      return 0;
   }
   uint32 FillerType;                            // Type of filler
   const char * FillerName = s.OpcodeDef->Name;  // Name of filler
   uint32 IFillerBegin = IBegin;                 // Start of filling space
   uint32 IFillerEnd;                            // End of filling space

   // check for CC = int 3 breakpoint, 3C00 = 90 NOP, 11F = multibyte NOP
   if (Opcodei == 0xCC || (Opcodei & 0xFFFE) == 0x3C00 || Opcodei == 0x11F) {
      // Instruction is a NOP or int 3 breakpoint
      FillerType = Opcodei;
   }
   else if (s.Warnings & 0x1000000) {
      // Instruction is a LEA, MOV, etc. with same source and destination
      // used as a multi-byte NOP
      FillerType = 0xFFFFFFFF;
   }
   else {
      // This instruction does something. Not a filler
      return 0;
   }
   // Save beginning position
   IFillerEnd = IEnd = IBegin;

   // Loop through instructions to find all consecutive fillers
   while (NextInstruction2()) {

      // Parse instruction
      ParseInstruction();

      // Check if code is filler
      if (!(OpcodeOptions & 0x40)) {
         // This instruction can not be a filler
         // Save position of this instruction
         IFillerEnd = IBegin;
         break;
      }
      if (Opcodei != 0xCC && (Opcodei & 0xFFFE) != 0x3C00 && Opcodei != 0x11F
      && !(s.Warnings & 0x1000000)) {
         // Not a filler
         // Save position of this instruction
         IFillerEnd = IBegin;
         break;
      }
      // If loop exits here then fillers end at end of this instruction
      IFillerEnd = IEnd;
   }
   // Safety check
   if (IFillerEnd <= IFillerBegin) return 0;

   // Size of fillers
   uint32 FillerSize = IFillerEnd - IFillerBegin;

   // Write size of filling space
   OutFile.Put(CommentSeparator);
   OutFile.Put("Filling space: ");
   if (IFillerEnd - IFillerBegin < 0x100) {
      // Write with 2 hexadecimal digits
      OutFile.PutHex(uint8(FillerSize), 1);
   }
   else {
      // Write with 8 hexadecimal digits
      OutFile.PutHex(FillerSize, 1);
   }
   OutFile.NewLine();
   // Write filler type
   OutFile.Put(CommentSeparator);
   OutFile.Put("Filler type: ");
   switch (FillerType) {
   case 0xCC:
      FillerName = "INT 3 Debug breakpoint";
      break;
   case 0x3C00:
      FillerName = "NOP";
      break;
   case 0x3C01:
      FillerName = "NOP with prefixes";
      break;
   case 0x011F:
      FillerName = "Multi-byte NOP";
      break;
   }
   OutFile.Put(FillerName);
   if (FillerType == 0xFFFFFFFF) {
      OutFile.Put(" with same source and destination");
   }

   // Write as bytes
   uint32 Pos;
   for (Pos = IFillerBegin; Pos < IFillerEnd; Pos++) {
      if ((Pos - IFillerBegin & 7) == 0) {
         // Start new line
         OutFile.NewLine();
         OutFile.Tabulate(AsmTab1);
         OutFile.Put("db ");
      }
      else {
         // Continue on same line
         OutFile.Put(", ");
      }
      // Write byte value
      OutFile.PutHex(Get<uint8>(Pos), 1);
   }
   // Blank line
   OutFile.NewLine();
   OutFile.NewLine();

   // Find alignment
   uint32 Alignment = 4;                         // Limit to 2^4 = 16

   // Check if first non-filler is aligned by this value
   while (Alignment && (IFillerEnd & (1 << Alignment) - 1)) {
      // Not aligned by 2^Alignment
      Alignment--;
   }
   if (Alignment) {

      // Check if smaller alignment would do
      if (Alignment > 3 && FillerSize < 1u << (Alignment-1)) {
         // End is aligned by 16, but there are less than 8 filler bytes.
         // Change to align 8
         Alignment--;
      }

      // Write align directive
      OutFile.Put("ALIGN");
      OutFile.Tabulate(AsmTab1);
      OutFile.PutDecimal(1 << Alignment, 1);     // 2 ^ Alignment
      OutFile.NewLine();
      // Prevent writing ALIGN again
      FlagPrevious &= ~1;
   }

   // Restore IBegin and IEnd to beginning of first non-filler instruction
   IBegin = IEnd = IFillerEnd;

   if (LabelInaccessible == IFillerBegin && IFillerEnd < LabelEnd) {
      // Mark first instruction after filler as inaccessible
      LabelInaccessible = IFillerEnd;
   }

   // Return success. Fillers have been written. Don't write as normal instructions
   return 1;
}

void CDisassembler::WriteFileBegin() {
   // Write begin of file

   // Initial comment
   OutFile.Put(CommentSeparator);
   OutFile.Put("Disassembly of file: ");
   OutFile.Put(cmd.InputFile);
   OutFile.NewLine();
   // Date and time. Note: will fail after year 2038
   time_t time1 = time(0);
   const char * timestring = ctime(&time1);
   if (timestring) {
      OutFile.Put(CommentSeparator);
      OutFile.Put(timestring);
      OutFile.NewLine();
   }

   // Write mode
   OutFile.Put(CommentSeparator);
   OutFile.Put("Mode: ");
   OutFile.PutDecimal(WordSize);
   OutFile.Put(" bits");
   OutFile.NewLine();

   // Write syntax dialect
   OutFile.Put(CommentSeparator);
   OutFile.Put("Syntax: ");
   OutFile.Put(WordSize < 64 ? "MASM/ML" : "ML64");
   OutFile.NewLine();

   // Write instruction set as comment
   // Instruction set is at least .386 if 32 bit mode
   if (InstructionSetMax < 3 && (MasmOptions & 0x200)) InstructionSetMax = 3;

   // Get name of basic instruction set
   const char * set0 = "";
   switch (InstructionSetMax) {
   case 0:
      set0 = "8086";  break;
   case 1:
      set0 = "80186";  break;
   case 2:
      set0 = "80286";  break;
   case 3:
      set0 = "80386";  break;
   case 4:
      set0 = "80486";  break;
   case 5:
      set0 = "Pentium";  break;
   case 6:
      set0 = "Pentium Pro";  break;
   case 7:
      set0 = "MMX";  break;
   case 8:
      set0 = "Pentium II";  break;
   case 0x11:
      set0 = "SSE";  break;
   case 0x12:
      set0 = "SSE2";  break;
   case 0x13:
      set0 = "SSE3";  break;
   case 0x14:
      set0 = "Supplementary SSE3";  break;
   case 0x15:
      set0 = "SSE4";  break;
   }
   // Write as comment
   OutFile.Put(CommentSeparator);
   OutFile.Put("Instruction set: ");
   OutFile.Put(set0);
   // Additional instruction sets:
   if (WordSize > 32) OutFile.Put(", x64");
   if (InstructionSetOR & 0x100) OutFile.Put(", 80x87");
   if (InstructionSetOR & 0x800) OutFile.Put(", privileged instructions");
   if (InstructionSetOR & 0x1000) OutFile.Put(", 3DNow");
   OutFile.NewLine();

   if (WordSize < 64) {
      // Write instruction set directive, except for 64 bit assembler

      const char * set1 = "";
      switch (InstructionSetMax) {
      case 0:
         set1 = ".8086";  break;
      case 1:
         set1 = ".186";  break;
      case 2:
         set1 = ".286";  break;
      case 3:
         set1 = ".386";  break;
      case 4:
         set1 = ".486";  break;
      case 5:
         set1 = ".586";  break;
      case 6: default:
         set1 = ".686";  break;
      }

      // Write basic instruction set
      OutFile.NewLine();
      OutFile.Put(set1);
      if (InstructionSetOR & 0x800) {
         // Privileged. Add "p"
         OutFile.Put("p");
      }
      OutFile.NewLine();

      // Write extended instruction set
      if (InstructionSetOR & 0x100) {
         // Floating point
         if (InstructionSetMax < 3) {
            OutFile.Put(".8087");  OutFile.NewLine();
         }
         else if (InstructionSetMax < 5) {
            OutFile.Put(".387");  OutFile.NewLine();
         }
      }
      if (InstructionSetMax >= 0x11) {
         // .xmm directive. Not differentiated between SSE, SSE2, etc.
         OutFile.Put(".xmm");  OutFile.NewLine();
      }
      else if (InstructionSetMax >= 7) {
         // .mmx directive
         OutFile.Put(".mmx");  OutFile.NewLine();
      }
   }
   if (MasmOptions & 1) {
      // Need dotname option
      OutFile.Put("option dotname");  OutFile.NewLine();
   }
   if (WordSize == 32) {
      // Write .model flat if 32 bit mode
      OutFile.Put(".model flat");  OutFile.NewLine();
   }
   // Initialize Assumes for segment registers
   if (!(MasmOptions & 0x100)) {
      // No 16-bit segments. Assume CS=DS=ES=SS=flat
      Assumes[0]=Assumes[1]=Assumes[2]=Assumes[3] = ASM_SEGMENT_FLAT;
   }
   else {
      // 16-bit segmented model. Segment register values unknown
      Assumes[0]=Assumes[1]=Assumes[2]=Assumes[3] = ASM_SEGMENT_UNKNOWN;
   }
   // FS and GS assumed to ERROR
   Assumes[4] = Assumes[5] = ASM_SEGMENT_ERROR;

   // Write assume if FS or GS used
   // This is superfluous because an assume directive will be written at first use of FS/GS
   if (MasmOptions & 2) {
      OutFile.Put("assume fs:nothing");  OutFile.NewLine();
   }
   if (MasmOptions & 4) {
      OutFile.Put("assume gs:nothing");  OutFile.NewLine();
   }
   // Blank line
   OutFile.NewLine();

   // Public and external symbols
   WritePublicsAndExternals();
}

void CDisassembler::WriteFileEnd() {
   // Write end of file
   OutFile.NewLine();
   OutFile.Put("END");
}

void CDisassembler::WriteSegmentBegin() {
   // Write begin of segment
   int AlignmentIncompatible = 0;                // != 0 if alignment not supported by assembler

   OutFile.NewLine();                            // Blank line

   // Check if Section is valid
   if (Section == 0 || Section >= Sections.GetNumEntries()) {
      // Illegal segment entry
      OutFile.Put("UNKNOWN SEGMENT");  OutFile.NewLine(); 
      return;
   }

   // Write segment name
   WriteSectionName(Section);
   // Tabulate
   OutFile.Put(" "); OutFile.Tabulate(AsmTab1);
   // Write "segment"
   OutFile.Put("SEGMENT ");
   // Write alignment
   switch (Sections[Section].Align) {
   case 0:  // 1
      OutFile.Put("BYTE ");  break;
   case 1:  // 2
      OutFile.Put("WORD ");  break;
   case 2:  // 4
      OutFile.Put("DWORD ");  break;
   case 3:  // 8
      AlignmentIncompatible = 8;
      // Cannot align by 8. Continue in next case
   case 4:  // 16
      OutFile.Put("PARA ");  break;
   case 8:  // 256 or 4096. Definition is ambiguous!
      OutFile.Put("PAGE ");  break;
   default:
      // Alignment not supported by assembler
      AlignmentIncompatible = 1 << Sections[Section].Align;
      OutFile.Put("PAGE ");  break;
   }
   if (WordSize != 64) {
      // "PUBLIC" not supported by ml64 assembler
      OutFile.Put("PUBLIC ");
      // Write segment word size if necessary
      if (MasmOptions & 0x100) {
         // There is at least one 16-bit segment. Write segment word size
         switch (Sections[Section].WordSize) {
         case 16:
            OutFile.Put("USE16 ");  break;
         case 32:
            OutFile.Put("USE32 ");  break;
         case 64:
            OutFile.Put("USE64 ");  break;
         }
      }
   }
   // Write segment class
   switch (Sections[Section].Type & 0xFF) {
   case 1:
       OutFile.Put("'CODE'");  break;
   case 2:
       OutFile.Put("'DATA'");  break;
   case 3:
       OutFile.Put("'BSS'");  break;
   case 4:
       OutFile.Put("'CONST'");  break;
   default:;
      // Unknown class. Write nothing
   }

   // Tabulate to comment
   OutFile.Put(" ");  OutFile.Tabulate(AsmTab3);
   OutFile.Put(CommentSeparator);
   // Write section number
   OutFile.Put("section number ");  
   OutFile.PutDecimal(Section);

   // New line
   OutFile.NewLine();

   if (Sections[Section].Type & 0x1000) {
      // Communal
      OutFile.Put(CommentSeparator);
      OutFile.Put(" Communal section not supported by MASM");
      OutFile.NewLine();
   }

   if (AlignmentIncompatible) {
      // Write warning for incompatible alignment
      OutFile.Put(CommentSeparator);
      OutFile.Put(" Alignment by ");
      OutFile.PutDecimal(AlignmentIncompatible);
      OutFile.Put(" not supported by MASM");
      OutFile.NewLine();
   }

   if (WordSize == 16 && Sections[Section].Type == 1) {
      // 16 bit code segment. Write ASSUME CS: SEGMENTNAME
      OutFile.Put("ASSUME ");
      OutFile.Tabulate(AsmTab1);
      OutFile.Put("CS:");
      if (Sections[Section].Group) {
         // Group name takes precedence over segment name
         WriteSectionName(Sections[Section].Group);
      }
      else {
         WriteSectionName(Section);
      }
      OutFile.NewLine();
      Assumes[1] = Section;
   }
}

void CDisassembler::WriteSegmentEnd() {
   // Write end of segment

   // Check if Section is valid
   if (Section == 0 || Section >= Sections.GetNumEntries()) {
      // Illegal segment entry
      OutFile.Put("UNKNOWN ENDS");  OutFile.NewLine(); 
      return;
   }

   // Write segment name
   const char * segname = NameBuffer.Buf() + Sections[Section].Name;
   OutFile.Put(segname);

   // Tabulate
   OutFile.Put(" "); OutFile.Tabulate(AsmTab1);
   // Write "segment"
   OutFile.Put("ENDS");
   // New line
   OutFile.NewLine();
}

void CDisassembler::WritePublicsAndExternals() {
   // Write public and external symbol definitions
   uint32 i;                                     // Loop counter
   uint32 LinesWritten = 0;                      // Count lines written
   char * XName;                                 // Name of external symbols

   // Loop through public symbols
   for (i = 0; i < Symbols.GetNumEntries(); i++) {
      if (Symbols[i].Scope & 0x1C) {
         // Symbol is public
         OutFile.Put("public ");
         // Write name
         OutFile.Put(Symbols.GetName(i));
         // Check if weak or communal
         if (Symbols[i].Scope & 0x18) {
            // Scope is weak or communal
            OutFile.Tabulate(AsmTab3);
            OutFile.Put(CommentSeparator);
            if (Symbols[i].Scope & 8) OutFile.Put("Note: Weak. Not supported by MASM ");
            if (Symbols[i].Scope & 0x10) OutFile.Put("Note: Communal. Not supported by MASM");
         }
         OutFile.NewLine();  LinesWritten++;
      }
   }
   // Blank line if anything written
   if (LinesWritten) {
      OutFile.NewLine();
      LinesWritten = 0;
   }
   // Loop through external symbols
   for (i = 0; i < Symbols.GetNumEntries(); i++) {

      if (Symbols[i].Scope & 0x20) {
         // Symbol is external
         OutFile.Put("extern ");
         // Get name
         XName = Symbols.GetName(i);
         // Check for dynamic import
         if (Symbols[i].DLLName && strncmp(XName, Symbols.ImportTablePrefix, (uint32)strlen(Symbols.ImportTablePrefix)) == 0) {
            // Remove "_imp" prefix from name
            XName += (uint32)strlen(Symbols.ImportTablePrefix);
         }

         // Write name
         OutFile.Put(XName);
         OutFile.Put(": ");

         // Write type
         if ((Symbols[i].Type & 0xFE) == 0x84) {
            // Far
            OutFile.Put("far");
         }
         else if ((Symbols[i].Type & 0xF0) == 0x80 || Symbols[i].DLLName) {
            // Near
            OutFile.Put("near");
         }
         else {
            // Data. Write size
            switch (GetDataItemSize(Symbols[i].Type)) {
            case 1: default:
               OutFile.Put("byte");  break;
            case 2:
               OutFile.Put("word");  break;
            case 4:
               OutFile.Put("dword");  break;
            case 6:
               OutFile.Put("fword");  break;
            case 8:
               OutFile.Put("qword");  break;
            case 10:
               OutFile.Put("tbyte");  break;
            case 16:
               OutFile.Put("xmmword");  break;
            }
         }
         // Add comment if DLL import
         if (Symbols[i].DLLName) {
            OutFile.Tabulate(AsmTab3);
            OutFile.Put(CommentSeparator);
            OutFile.Put(Symbols.GetDLLName(i));
         }
         // Finished line
         OutFile.NewLine();  LinesWritten++;
      }
   }
   // Blank line if anything written
   if (LinesWritten) {
      OutFile.NewLine();
      LinesWritten = 0;
   }
   // Write the value of any exported constants
   // Loop through external symbols
   for (i = 0; i < Symbols.GetNumEntries(); i++) {
      if (Symbols[i].Section == ASM_SEGMENT_ABSOLUTE && (Symbols[i].Scope & 0x1C)) {
         // Symbol is public and constant
         // Write name
         OutFile.Put(Symbols.GetName(i));
         OutFile.Put(" equ ");
         // Write value as hexadecimal
         OutFile.PutHex(Symbols[i].Offset, 1);
         // Write decimal value as comment
         OutFile.Tabulate(AsmTab3);
         OutFile.Put(CommentSeparator);
         OutFile.PutDecimal(Symbols[i].Offset, 1);
         OutFile.NewLine();  LinesWritten++;
      }
   }
   // Blank line if anything written
   if (LinesWritten) {
      OutFile.NewLine();
      LinesWritten = 0;
   }
   // Write any group definitions
   int32 GroupId, SegmentId;
   // Loop through sections to search for group definitions
   for (GroupId = 1; GroupId < (int32)Sections.GetNumEntries(); GroupId++) {

      // Get section type
      uint32 SectionType = Sections[GroupId].Type;
      if (SectionType & 0x800) {
         // This is a segment group definition
         // Count number of members
         uint32 NumMembers = 0;
         // Write group name
         WriteSectionName(GroupId);
         // Write "group"
         OutFile.Put(" ");  OutFile.Tabulate(AsmTab1);  OutFile.Put("GROUP ");
         // Search for group members
         for (SegmentId = 1; SegmentId < (int32)Sections.GetNumEntries(); SegmentId++) {
            if (Sections[SegmentId].Group == GroupId && !(Sections[SegmentId].Type & 0x800)) {
               // is this first member?
               if (NumMembers++) {
                  // Not first member. Write comma
                  OutFile.Put(", ");
               }
               // Write group member
               WriteSectionName(SegmentId);
            }
         }
         // End line
         OutFile.NewLine();  LinesWritten++;
      }
   }
   // Blank line if anything written
   if (LinesWritten) {
      OutFile.NewLine();
      LinesWritten = 0;
   }
}

void CDisassembler::WriteFunctionBegin() {
   // Write begin of function IFunction

   // Check if IFunction is valid
   if (IFunction == 0 || IFunction >= FunctionList.GetNumEntries()) {
      // Should not occur
      OutFile.Put(CommentSeparator);
      OutFile.Put("Internal error: undefined function begin");
      return;
   }

   // Get symbol old index
   uint32 symi = FunctionList[IFunction].OldSymbolIndex;

   // Get symbol record
   uint32 SymI = Symbols.Old2NewIndex(symi);

   // check scope
   if (Symbols[SymI].Scope & 0x1C) {
      // Has public scope. Write PROC entry
      // (Will be written as label if not public)
       OutFile.NewLine();                        // Blank line

      // Remember that symbol has been written
      Symbols[SymI].Scope |= 0x100;

      // Check alignment if preceded by NOP
      if ((FlagPrevious & 1) && (IBegin & 0x0F) == 0 && Sections[Section].Align >= 4) {
          OutFile.Put("ALIGN");  OutFile.Tabulate(AsmTab1);  OutFile.Put("16");       
          OutFile.NewLine();
      }

      // Write name
      WriteSymbolName(SymI);
      // Space
      OutFile.Put(" "); OutFile.Tabulate(AsmTab1);
      // Write "PROC"
      OutFile.Put("PROC");
      // Write "NEAR" unless 64 bit mode
      if (WordSize < 64) OutFile.Put(" NEAR");

      // Check if weak
      if (Symbols[SymI].Scope & 8) {
         OutFile.NewLine();
         OutFile.Put(CommentSeparator);
         OutFile.Put(" WEAK ");
         WriteSymbolName(SymI);
      }
      // Check if communal
      if (Symbols[SymI].Scope & 0x10) {
         OutFile.NewLine();
         OutFile.Put(CommentSeparator);
         OutFile.Put(" COMDEF ");
         WriteSymbolName(SymI);
      }
   // End line
   OutFile.NewLine();
   }
}

void CDisassembler::WriteFunctionEnd() {
   // Write end of function

   // Check if IFunction is valid
   if (IFunction == 0 || IFunction >= FunctionList.GetNumEntries()) {
      // Should not occur
      OutFile.Put(CommentSeparator);
      OutFile.Put("Internal error: undefined function begin");
      return;
   }

   // Get symbol index
   uint32 SymOldI = FunctionList[IFunction].OldSymbolIndex;
   uint32 SymNewI = Symbols.Old2NewIndex(SymOldI);

   // check scope
   if (Symbols[SymNewI].Scope & 0x1C) {
      // Has public scope. Write ENDP
      // Write name
      WriteSymbolName(SymNewI);
      // Space
      OutFile.Put(" "); OutFile.Tabulate(AsmTab1);
      // Write "ENDP"
      OutFile.Put("ENDP");
      OutFile.NewLine();
   }
}

void CDisassembler::WriteCodeLabel(uint32 symi) {
   // Write private or public code label. symi is new symbol index

   // Get scope
   uint32 Scope = Symbols[symi].Scope;

   // Check scope
   if (Scope & 0x100) return;                    // Has been written as PROC

   if (Scope == 0) {
      // Inaccessible. No name. Make blank line
      OutFile.NewLine();
      // Remember position for warning check
      LabelInaccessible = IBegin;
      return;
   }

   // Begin on new line if preceded by another symbol
   if (OutFile.GetColumn()) OutFile.NewLine(); 

   if ((Scope & 0xFF) > 1) {
      // Scope > function local. Write as label near
      // Check if extra linefeed needed
      // if (!(IFunction && FunctionList[IFunction].Start == IBegin)) 
      // New line
      OutFile.NewLine();

      // Check alignment if preceded by NOP
      if ((FlagPrevious & 1) && (IBegin & 0x0F) == 0 && Sections[Section].Align >= 4) {
          OutFile.Put("ALIGN");  OutFile.Tabulate(AsmTab1);  OutFile.Put("16");       
          OutFile.NewLine();
      }

      // Write name
      WriteSymbolName(symi);
      // Space
      OutFile.Put(" "); OutFile.Tabulate(AsmTab1);
      // Write "LABEL"
      OutFile.Put("LABEL");
      // Write "NEAR" even 64 bit mode
      OutFile.Put(" NEAR");
      // New line
      OutFile.NewLine();

      // Check if weak
      if (Symbols[symi].Scope & 8) {
         OutFile.Put(CommentSeparator);
         OutFile.Put(" WEAK ");
         WriteSymbolName(symi);
         OutFile.NewLine();
      }
      // Check if communal
      if (Symbols[symi].Scope & 0x10) {
         OutFile.Put(CommentSeparator);
         OutFile.Put(" COMDEF ");
         WriteSymbolName(symi);
         OutFile.NewLine();
      }
   }
   else {
      // Symbol is local to current function. Write name with colon

      if (FlagPrevious & 2) {
         // Insert blank line if previous instruction was unconditional jump or return
         OutFile.NewLine();
      }

      // Write name
      WriteSymbolName(symi);
      // Write ":"
      OutFile.Put(":");
      if (OutFile.GetColumn() > AsmTab1) {
         // Past tabstop. Go to next line
         OutFile.NewLine();                   // New line
      }
   }
   // Remember this has been written
   Symbols[symi].Scope |= 0x100;
}

void CDisassembler::WriteAssume() {
   // Write assume directive for segment register
   if (!s.AddressField) return;

   int32 SegReg, PrefixSeg;                      // Segment register used
   uint32 symo;                                  // Target symbol old index
   uint32 symi;                                  // Target symbol new index
   int32 TargetSegment;                          // Target segment/section
   int32 TargetGroup;                            // Group containing target segment

   // Find which segment register is used for addressing memory operand
   SegReg = 3;  // DS is default
   if (s.BaseReg == 4+1 || s.BaseReg == 5+1) {
      // Base register is (E)BP or ESP
      SegReg = 2;     // SS register used unless there is a prefix
   }
   if (s.Prefixes[0]) {
      // There is a segment prefix
      PrefixSeg = GetSegmentRegisterFromPrefix();
      if (PrefixSeg >= 0 && PrefixSeg <= 5) {
         // Segment prefix is valid. Segment determined by segment prefix
         SegReg = PrefixSeg;
      }
   }
   // Default target segment is none
   TargetSegment = TargetGroup = 0;

   // Find symbol referenced by next instruction
   if (s.AddressRelocation && s.AddressRelocation < Relocations.GetNumEntries()) {
      symo = Relocations[s.AddressRelocation].TargetOldIndex; // Target symbol old index
      if (symo) {
         symi = Symbols.Old2NewIndex(symo);                   // Target symbol new index
         if (symi) {
            TargetSegment = Symbols[symi].Section;            // Target segment
            if (TargetSegment < 0 || TargetSegment >= (int32)Sections.GetNumEntries()) {
               TargetSegment = 0;
            }
            else {
               TargetGroup = Sections[TargetSegment].Group;   // Group containing target segment
               if (TargetGroup <= ASM_SEGMENT_ERROR || TargetGroup >= (int32)Sections.GetNumEntries()) {
                  TargetGroup = 0;
               }
            }
         }
      }
   }
   if (TargetSegment) {
      // Target has a segment. Check if it is different from currently assumed segment
      if (TargetSegment != Assumes[SegReg] && TargetGroup != Assumes[SegReg]) {
         // Assume directive needed
         // If segment belongs to a group then the group takes precedence
         if (TargetGroup) TargetSegment = TargetGroup;
         // Write assume directive
         OutFile.Put("ASSUME ");
         OutFile.Tabulate(AsmTab1);
         OutFile.Put(RegisterNamesSeg[SegReg]);  // Name of segment register used
         OutFile.Put(":");
         WriteSectionName(TargetSegment);        // Name of segment or group referenced
         OutFile.NewLine();
         Assumes[SegReg] = TargetSegment;
      }
   }
   else {
      // Target segment not specified. Assumed value may be anyting but 'error'
      if (Assumes[SegReg] <= ASM_SEGMENT_ERROR) {
         // Segment register is assumed to 'error'. Change assume to 'nothing'
         OutFile.Put("ASSUME ");
         OutFile.Tabulate(AsmTab1);
         OutFile.Put(RegisterNamesSeg[SegReg]);  // Name of segment register used
         OutFile.Put(":NOTHING");
         OutFile.NewLine();
         Assumes[SegReg] = ASM_SEGMENT_NOTHING;
      }
   }
}

void CDisassembler::WriteInstruction() {
   // Write instruction and operands
   uint32 NumOperands = 0;                       // Number of operands written
   uint32 i;                                     // Loop index
   const char * OpName;                          // Opcode name
   const char * OpComment;                       // Comment for opcode

   if (s.AddressFieldSize) {
      // There is a memory operand. Check if ASSUME directive needed
      WriteAssume();
   }

   if (CodeMode & 6) {
      // Code is dubious. Show as comment only
      OutFile.Put(CommentSeparator);             // Start comment
   }

   if ((s.Operands[0] & 0xF0) == 0xC0 || (s.Operands[1] & 0xF0) == 0xC0) {
      // String instruction or xlat instruction
      WriteStringInstruction();
      return;
   }

   OutFile.Tabulate(AsmTab1);                    // Tabulate

   if (s.Prefixes[2]) {
      OutFile.Put("lock ");                      // Lock prefix
   }

   // Get opcode name
   if (s.OpcodeDef->Name) {
      // Opcode name
      OpName = s.OpcodeDef->Name;
      // Search for opcode comment
      OpComment = strchr(OpName, ';');
   }
   else {
      OpName = "UNDEFINED";                      // Undefined code with no name
      OpComment = 0;
   }
   // Write opcode name
   if (OpComment) {
      // OpName string contains opcode name and comment, separated by ';'
      while (*OpName != ';' && *OpName != 0) {   // Write opcode name until comment
         OutFile.Put(*(OpName++));
      }
   }
   else {
      OutFile.Put(OpName);                       // Write normal opcode name
   }

   // Check suffix option
   if (s.OpcodeDef->Options & 1) {
      // Append suffix for operand size or type to name
      if (s.OpcodeDef->AllowedPrefixes & 0xE00) {
         // Operand type defined by prefixes
         switch (s.Prefixes[5]) {
         case 0:     // No prefix = ps
            OutFile.Put("ps");  break;
         case 0x66:  // 66 prefix = pd
            OutFile.Put("pd");  break;
         case 0xF3:  // F3 prefix = ss
            OutFile.Put("ss");  break;
         case 0xF2:  // F2 prefix = sd
            OutFile.Put("sd");  break;
         default:
            err.submit(9000); // Should not occur
         }
      }
      else {
         // Suffix for operand size
         i = s.OperandSize / 8;
         if (i <= 8) {
            static const char SizeSuffixes[] = " bw d f q"; // Table of suffixes
            OutFile.Put(SizeSuffixes[i]);
         }
      }
   }

   // Space between opcode name and operands
   OutFile.Put(" "); OutFile.Tabulate(AsmTab2);  // Tabulate. At least one space

   // Write operands
   for (i = 0; i < 3; i++) {
      if (s.Operands[i]) {

         // Write operand i
         if (NumOperands++) {
            // At least one operand before this one. Separate by ", "
            OutFile.Put(", ");
         }
         if (s.Operands[i] & 0x60000) {
            WriteRMOperand(s.Operands[i]);       // Operand indicated by mod/rm bits
         }
         else if (s.Operands[i] & 0x10000) {
            WriteRegOperand(s.Operands[i]);      // Operand indicated by reg bits
         }
         else if ((s.Operands[i] & 0x3000) || (s.Operands[i] & 0xF0) == 0x80) {
            WriteImmediateOperand(s.Operands[i]);// Immediate or jump/call operand
         }
         else if (s.Operands[i]) {
            WriteOtherOperand(s.Operands[i]);    // Other type of operand
         }
      }
   }
   if (OpComment) {
      // Write opcode comment
      OutFile.Put(' ');
      OutFile.Put(CommentSeparator);
      OutFile.Put(OpComment + 1);
   }
}

void CDisassembler::WriteStringInstruction() {
   // Write string instruction or xlat instruction
   uint32 NumOperands = 0;                       // Number of operands written
   uint32 i;                                     // Loop index

   if (!(s.OpcodeDef->AllowedPrefixes & 0x1100)) {
      // Operand size is 8 if operand size prefixes not allowed
      s.OperandSize = 8;
   }

   OutFile.Tabulate(AsmTab1);                    // Tabulate

   // Check repeat prefix
   if (s.OpcodeDef->AllowedPrefixes & 0x20) {
      if (s.Prefixes[3]) {
         // Repeat prefix
         OutFile.Put("rep ");
      }
   }
   else if (s.OpcodeDef->AllowedPrefixes & 0x40) {
      if (s.Prefixes[3] == 0xF2) {
         // repne prefix
         OutFile.Put("repne ");
      }
      else if (s.Prefixes[3] == 0xF3) {
         // repe prefix
         OutFile.Put("repe ");
      }
   }

   // Write opcode name
   OutFile.Put(s.OpcodeDef->Name);               // Opcode name

   if (((s.OpcodeDef->AllowedPrefixes & 4) && s.Prefixes[0]) 
   || ((s.OpcodeDef->AllowedPrefixes & 1) && s.Prefixes[1])) {
      // Has segment or address size prefix. Must write operands explicitly
      OutFile.Put(" ");                          // Space before operands

      // Check address size for pointer registers
      const char * * PointerRegisterNames;
      switch (s.AddressSize) {
      case 16:
         PointerRegisterNames = RegisterNames16;  break;
      case 32:
         PointerRegisterNames = RegisterNames32;  break;
      case 64:
         PointerRegisterNames = RegisterNames64;  break;
      }

      // Loop for possibly two operands
      for (i = 0; i < 2; i++) {
         if (s.Operands[i]) {
            // Operand i defined
            if (NumOperands++) {
               // An operand before this one. Separate by ", "
               OutFile.Put(", ");
            }

            if (NumOperands == 1) {
               // Write operand size for first operand
               switch (s.OperandSize) {
               case 8:
                  OutFile.Put("byte ptr ");  break;
               case 16:
                  OutFile.Put("word ptr ");  break;
               case 32:
                  OutFile.Put("dword ptr ");  break;
               case 64:
                  OutFile.Put("qword ptr ");  break;
               }
            }

            // Get segment
            uint32 Segment = 1;                  // Default segment is DS
            if (s.Prefixes[0]) {
               Segment = GetSegmentRegisterFromPrefix(); // Interpret segment prefix
            }
            if ((s.Operands[i] & 0xCF) == 0xC2) {
               Segment = 0;                      // Segment is ES regardless of prefix for [edi] operand
            }

            // Write segment override
            OutFile.Put(RegisterNamesSeg[Segment]);
            OutFile.Put(":");

            // Opening "["
            OutFile.Put("[");

            // Write pointer register
            switch (s.Operands[i] & 0xCF) {
            case 0xC0:  // [bx], [ebx] or [rbx]
               OutFile.Put(PointerRegisterNames[3]);  
               break;
            case 0xC1:  // [si], [esi] or [rsi]
               OutFile.Put(PointerRegisterNames[6]);  
               break;
            case 0xC2:  // [di], [edi] or [rdi]
               OutFile.Put(PointerRegisterNames[7]);  
               break;
            }

            // Closing "]"
            OutFile.Put("]");
         }
      }
   }
   else {
      // No segment prefix. We don't have to write the operands
      // Append suffix for operand size, except for xlat
      if ((s.Operands[1] & 0xCF) != 0xC0) {

         // Suffix for operand size
         uint32 i = s.OperandSize / 8;
         if (i <= 8) {
            static const char SizeSuffixes[] = " bw d   q"; // Table of suffixes
            OutFile.Put(SizeSuffixes[i]);
         }
      }
   }
}

void CDisassembler::WriteCodeComment() {
   // Write hex listing of instruction as comment after instruction
   uint32 i;                                     // Index to current byte
   uint32 FieldSize;                             // Number of bytes in field
   const char * Spacer;                          // Space between fields

   OutFile.Tabulate(AsmTab3);                    // Tabulate to comment field
   OutFile.Put(CommentSeparator);                // Start comment

   // Write address
   if (SectionEnd + SectionAddress + (uint32)ImageBase > 0xFFFF) {
      // Write 32 bit address
      OutFile.PutHex(IBegin + SectionAddress + (uint32)ImageBase);
   }
   else {
      // Write 16 bit address
      OutFile.PutHex((uint16)(IBegin + SectionAddress));
   }

   // Space after address
   OutFile.Put(" _");

   // Start of instruction
   i = IBegin;

   // Write bytes
   while (i < IEnd) {
      FieldSize = 1;                             // Size of field to write
      Spacer = " ";                              // Space between fields

      // Spacer and FieldSize depends on fields
      if (i == s.OpcodeStart1 && i > IBegin) {
         Spacer = ": ";                          // Space between prefixes and opcode
      }
      if (i == s.OpcodeStart2 + 1) {
         Spacer = ". ";                          // Space between opcode and mod/reg/rm bytes
      }
      if (i == s.AddressField && s.AddressFieldSize) {
         Spacer = ", ";                          // Space before address field
         FieldSize = s.AddressFieldSize;
      }
      if (i == s.ImmediateField && s.ImmediateFieldSize) {
         Spacer = ", ";                          // Space before immediate operand field
         FieldSize = s.ImmediateFieldSize;
      }
      // Write space
      OutFile.Put(Spacer);

      // Write byte or bytes
      switch (FieldSize) {
      case 1:  // Write single byte
         OutFile.PutHex(Get<uint8>(i));
         break;
      case 2:  // Write two bytes
         OutFile.PutHex(Get<uint16>(i));
         break;
      case 3:  // Write three bytes (operands for "enter" instruction)
         OutFile.PutHex(Get<uint16>(i));
         OutFile.Put(", ");
         OutFile.PutHex(Get<uint8>(i+2));
         break;
      case 4:  // Write four bytes
         if ((s.Operands[0] & 0xFE) == 0x84) {
            // Far jump/call address
            OutFile.PutHex(Get<uint16>(i));
            OutFile.Put(" ");
            OutFile.PutHex(Get<uint16>(i+2));
         }
         else {
            // Any other 32 bit operand
            OutFile.PutHex(Get<uint32>(i));
         }
         break;
      case 6:  // Write six bytes (far jump address)
         OutFile.PutHex(Get<uint32>(i));
         OutFile.Put(" ");
         OutFile.PutHex(Get<uint16>(i+4));
         break;
      case 8:  // Write eight bytes
         OutFile.PutHex(Get<uint64>(i));
         break;
      }
      // Search for relocation
      SARelocation rel1;                            // Make relocation records for searching
      rel1.Section = Section;
      rel1.Offset  = i;                             // rel1 marks current field in instruction

      // Is there a relocation source exactly here?
      int32 irel = Relocations.Exists(rel1);        // Finds relocation with source = i

      if (irel > 0) {
         // This field has a relocation. Indicate relocation type
         // 0 = unknown, 1 = direct, 2 = self-relative, 3 = image-relative, 
         // 4 = segment relative, 5 = relative to arbitrary ref. point, 8 = segment address/descriptor
         uint32 RelType = Relocations[irel].Type;
         if (RelType) {
            OutFile.Put(Lookup(RelocationTypeNames, RelType));
         }
         if (Relocations[irel].Size > FieldSize) {
            // Relocation has wrong size
            OutFile.Put(" Misplaced relocation.");
         }
      }

      // Point to next byte
      i += FieldSize;
   }
   // New line
   OutFile.NewLine();
}
