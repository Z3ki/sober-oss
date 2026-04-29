// ===== entry @ 00280b80 =====

/* WARNING: Control flow encountered bad instruction data */

void processEntry entry(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _FINI_0 @ 00280c20 =====

/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Instruction at (ram,0x00280c25) overlaps instruction at (ram,0x00280c24)
    */
/* WARNING: Removing unreachable block (ram,0x00280cc8) */

undefined8
_FINI_0(int *param_1,undefined1 *param_2,long param_3,undefined8 param_4,long param_5,
       undefined8 param_6,undefined8 param_7)

{
  byte *pbVar1;
  int *piVar2;
  undefined1 uVar3;
  int iVar4;
  code *pcVar5;
  byte bVar6;
  int *in_RAX;
  undefined1 unaff_BL;
  byte unaff_BH;
  undefined6 unaff_0000001a;
  undefined8 unaff_RBP;
  bool bVar7;
  
  bVar7 = ((ulong)in_RAX & 0xa3) != 0;
  if (bVar7 && -1 < (char)in_RAX) {
    *param_1 = (int)in_RAX;
    pbVar1 = (byte *)(param_3 + -0x3a);
    bVar6 = *pbVar1;
    *pbVar1 = *pbVar1 + unaff_BH;
    LOCK();
    uVar3 = *(undefined1 *)(param_5 + -0x1dc985e0);
    *(undefined1 *)(param_5 + -0x1dc985e0) = (char)unaff_RBP;
    UNLOCK();
    iVar4 = *(int *)(CONCAT71((int7)((ulong)unaff_RBP >> 8),uVar3) + 0x44551d92);
    *(undefined1 *)(param_1 + 1) = *param_2;
    iRam76e37fa3a85372df = ((int)in_RAX - iVar4) - (uint)CARRY1(bVar6,unaff_BH);
                    /* WARNING: Bad instruction - Truncating control flow here */
    halt_baddata();
  }
  if (!bVar7) {
    piVar2 = (int *)(CONCAT62(unaff_0000001a,CONCAT11(0x57,unaff_BL)) + 0x26);
    *piVar2 = *piVar2 + (int)param_3;
    do {
      bVar6 = DAT_ffffffffa8cf12d6 & (byte)param_3;
    } while ((char)bVar6 < '\x01');
    if ('\0' < (char)bVar6) {
                    /* WARNING: Bad instruction - Truncating control flow here */
      halt_baddata();
    }
    *in_RAX = *in_RAX + 1;
    return 0x31;
  }
  pcVar5 = (code *)swi(0xf5);
  (*pcVar5)();
  return param_7;
}



// ===== _INIT_1 @ 00280c60 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_1(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_2 @ 00281420 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_2(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_0 @ 00290a00 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_0(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_3 @ 002beb60 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_3(void)

{
  int *in_RAX;
  long in_RCX;
  int unaff_EBP;
  undefined1 auStack_8 [8];
  
  *in_RAX = *in_RAX - unaff_EBP;
  *(int *)(in_RCX + -0x61) = *(int *)(in_RCX + -0x61) - (int)auStack_8;
  *(byte *)((long)in_RAX * 3) = *(byte *)((long)in_RAX * 3) ^ (byte)((ulong)in_RAX >> 8);
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_4 @ 002db220 =====

/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Instruction at (ram,0x002db221) overlaps instruction at (ram,0x002db220)
    */
/* WARNING: Removing unreachable block (ram,0x002db221) */
/* WARNING: Removing unreachable block (ram,0x002db1e4) */

void _INIT_4(char *param_1,long param_2,undefined8 param_3,long param_4)

{
  ulong *puVar1;
  int *piVar2;
  bool bVar3;
  char cVar4;
  ulong in_RAX;
  undefined2 uVar6;
  undefined4 *puVar7;
  undefined1 unaff_BL;
  undefined7 unaff_00000019;
  ulong unaff_R14;
  bool bVar8;
  bool bVar9;
  byte in_OF;
  long lVar5;
  
  piVar2 = (int *)(CONCAT71(unaff_00000019,unaff_BL) + 0x56c2b27c);
  *piVar2 = *piVar2 << 0x18;
  bVar3 = (bool)(in_OF & 1);
  bVar9 = (in_RAX & 0x8000) != 0;
  bVar8 = (in_RAX & 0x4000) != 0;
  puVar7 = (undefined4 *)CONCAT71((int7)((ulong)param_3 >> 8),0x41);
  uVar6 = SUB82(puVar7,0);
  cVar4 = in(uVar6);
  *param_1 = cVar4;
  if ((in_RAX & 0x400) == 0) {
    cVar4 = (char)in_RAX + -0x24;
    lVar5 = CONCAT71((int7)(in_RAX >> 8),cVar4);
    *puVar7 = (int)(param_1 + 5);
    *(int *)(param_1 + 0x80) = *(int *)(param_1 + 0x80) - (int)lVar5;
    param_1[5] = cVar4;
    out(*(undefined4 *)(param_2 + 4),uVar6);
    puVar1 = (ulong *)(lVar5 + 9);
    *puVar1 = *puVar1 | unaff_R14;
                    /* WARNING: Bad instruction - Truncating control flow here */
    halt_baddata();
  }
  if (bVar8 || bVar3 != bVar9) {
    if (bVar8 || bVar3 != bVar9) {
      do {
      } while (bVar8 || bVar3 != bVar9);
      return;
    }
    do {
      param_1 = param_1 + 1;
      if (param_4 == 0) {
        halt_baddata();
      }
      param_4 = param_4 + -1;
    } while ((char)in_RAX == *param_1);
                    /* WARNING: Bad instruction - Truncating control flow here */
    halt_baddata();
  }
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_5 @ 0030c320 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_5(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_6 @ 003432e0 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_6(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_7 @ 0037a0b0 =====

void _INIT_7(undefined8 param_1,uint *param_2,undefined8 param_3,char *param_4)

{
  code *pcVar1;
  
  pcVar1 = (code *)swi(1);
  (*pcVar1)(param_1,param_2 + 1,
            CONCAT71((int7)(((ulong)*param_2 | 0x365f3069) >> 8),
                     (char)((ulong)*param_2 | 0x365f3069) + *param_4));
  return;
}



// ===== _INIT_8 @ 0037a0d0 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_8(void)

{
  long in_RCX;
  long unaff_RBP;
  undefined2 in_ES;
  
  *(undefined2 *)(unaff_RBP + 0x77) = in_ES;
  *(byte *)(in_RCX + -0x2a3a8049) = *(byte *)(in_RCX + -0x2a3a8049) ^ 0x99;
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_9 @ 0037a1b0 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_9(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_10 @ 0037a1d0 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_10(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_11 @ 0037ac60 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_11(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_12 @ 0037ac80 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_12(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_13 @ 00386060 =====

/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Instruction at (ram,0x00386072) overlaps instruction at (ram,0x00386071)
    */
/* WARNING: Removing unreachable block (ram,0x003860af) */
/* WARNING: Removing unreachable block (ram,0x003860b4) */
/* WARNING: Removing unreachable block (ram,0x00386072) */
/* WARNING: Removing unreachable block (ram,0x0038608f) */

void _INIT_13(undefined8 param_1,byte *param_2,undefined2 param_3,ulong param_4)

{
  long unaff_RBX;
  
  *param_2 = *param_2 | (byte)param_3;
  *(uint *)(unaff_RBX + -0x2b9842a0) = ~*(uint *)(unaff_RBX + -0x2b9842a0);
  out(param_3,(char)(param_4 & 0xffffffff));
  out(param_3,(int)CONCAT71((int7)((param_4 & 0xffffffff) >> 8),DAT_894fd4081bcf70a3));
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_14 @ 0038f510 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_14(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_15 @ 003934c0 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_15(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_16 @ 003944d0 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_16(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_17 @ 003a1fa0 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_17(undefined8 param_1,undefined4 param_2,long param_3)

{
  LOCK();
  *(undefined4 *)(param_3 + 0x1c5be6ca) = param_2;
  UNLOCK();
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_18 @ 003a5470 =====

/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Instruction at (ram,0x003a5472) overlaps instruction at (ram,0x003a5471)
    */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void _INIT_18(undefined8 param_1,undefined8 param_2,long param_3,undefined8 param_4)

{
  uint *puVar1;
  float fVar2;
  uint in_EAX;
  int in_register_00000004;
  undefined2 uVar3;
  uint uVar4;
  undefined2 in_ES;
  byte in_CF;
  byte in_PF;
  byte in_AF;
  byte in_ZF;
  byte in_SF;
  byte in_TF;
  byte in_IF;
  byte in_OF;
  byte in_NT;
  byte in_AC;
  byte in_VIF;
  byte in_VIP;
  byte in_ID;
  longdouble in_ST0;
  longdouble in_ST1;
  longdouble in_ST2;
  longdouble in_ST3;
  longdouble in_ST4;
  longdouble in_ST5;
  longdouble in_ST6;
  longdouble in_ST7;
  
  uVar3 = (undefined2)param_4;
  while( true ) {
    fVar2 = *(float *)(CONCAT62((int6)((ulong)param_4 >> 0x10),uVar3) + -0x67);
    *(ulong *)register0x00000020 =
         (ulong)(in_NT & 1) * 0x4000 | (ulong)(in_OF & 1) * 0x800 | (ulong)(in_IF & 1) * 0x200 |
         (ulong)(in_TF & 1) * 0x100 | (ulong)(in_SF & 1) * 0x80 | (ulong)(in_ZF & 1) * 0x40 |
         (ulong)(in_AF & 1) * 0x10 | (ulong)(in_PF & 1) * 4 | (ulong)(in_CF & 1) |
         (ulong)(in_ID & 1) * 0x200000 | (ulong)(in_VIP & 1) * 0x100000 |
         (ulong)(in_VIF & 1) * 0x80000 | (ulong)(in_AC & 1) * 0x40000;
    _DAT_ffffffffe65c6cf1 = (short)ROUND(in_ST0 * (longdouble)fVar2);
    if ((POPCOUNT((byte)in_EAX & 0x60) & 1U) != 0) {
                    /* WARNING: Bad instruction - Truncating control flow here */
      halt_baddata();
    }
    puVar1 = (uint *)((CONCAT44(in_register_00000004,in_EAX) & 0xffffffffffffff60) + 0x78);
    uVar4 = (uint)param_3;
    in_CF = *puVar1 < uVar4;
    in_OF = SBORROW4(*puVar1,uVar4);
    *puVar1 = *puVar1 - uVar4;
    in_SF = (int)*puVar1 < 0;
    in_ZF = *puVar1 == 0;
    in_PF = (POPCOUNT(*puVar1 & 0xff) & 1U) == 0;
    if ((bool)in_ZF || in_OF != in_SF) break;
    param_3 = (long)in_register_00000004 >> 0x1f;
    in_EAX = in_EAX & 0xffffff60;
    uVar3 = in_ES;
    register0x00000020 = (BADSPACEBASE *)((long)register0x00000020 + 8);
    in_ST0 = in_ST1;
    in_ST1 = in_ST2;
    in_ST2 = in_ST3;
    in_ST3 = in_ST4;
    in_ST4 = in_ST5;
    in_ST5 = in_ST6;
    in_ST6 = in_ST7;
  }
  out((short)param_3,in_EAX & 0xffffff60);
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_19 @ 004026b0 =====

/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Instruction at (ram,0x0040271c) overlaps instruction at (ram,0x0040271b)
    */
/* WARNING: Removing unreachable block (ram,0x0040271d) */
/* WARNING: Removing unreachable block (ram,0x00402732) */

undefined1  [16] _INIT_19(byte *param_1,undefined4 *param_2,undefined8 param_3,uint param_4)

{
  undefined4 uVar1;
  ulong in_RAX;
  undefined8 extraout_RDX;
  undefined8 *puVar2;
  uint unaff_EBP;
  undefined4 *puVar3;
  bool bVar4;
  undefined1 auVar5 [16];
  char unaff_retaddr;
  
  if ((in_RAX & 0x2ee62f57) != 0) {
    *(undefined4 *)param_1 = *param_2;
                    /* WARNING: Bad instruction - Truncating control flow here */
    halt_baddata();
  }
  puVar3 = param_2 + 1;
  out(*param_2,(short)CONCAT71((uint7)(in_RAX >> 8) & 0xffffff,0xe5));
  DAT_596b51a4 = DAT_596b51a4 - unaff_retaddr;
  func_0x3d6a46f9();
  uVar1 = in(0x43);
  bVar4 = param_4 + unaff_EBP != 0;
  if (!CARRY4(param_4,unaff_EBP) && bVar4) {
    if (bVar4) {
      in(0x9b);
      *(undefined1 *)
       (ulong)CONCAT22((short)((uint)uVar1 >> 0x10),
                       CONCAT11((byte)((uint)uVar1 >> 8) | *param_1,(char)uVar1)) =
           *(undefined1 *)puVar3;
      do {
        puVar2 = (undefined8 *)((long)register0x00000020 + -8);
        register0x00000020 = (BADSPACEBASE *)((long)register0x00000020 + -8);
        *puVar2 = 0x661e1548;
      } while( true );
    }
    auVar5._0_8_ = CONCAT71((int7)((ulong)param_1 >> 8),uRam30576fdf9ddf9a2f) & 0xffffffff;
    out(*(undefined1 *)puVar3,(short)extraout_RDX);
    auVar5._8_8_ = extraout_RDX;
    return auVar5;
  }
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_20 @ 00403480 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_20(undefined4 *param_1,undefined8 param_2,long param_3,long param_4)

{
  undefined4 in_EAX;
  undefined4 in_register_00000004;
  undefined2 in_ES;
  
  *(undefined2 *)(CONCAT44(in_register_00000004,in_EAX) + param_4) = in_ES;
  *param_1 = CONCAT31((int3)((uint)in_EAX >> 8),(byte)in_EAX & (&stack0x00000000)[param_3 * 2]);
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_21 @ 00404a40 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_21(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_22 @ 00404ff0 =====

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void _INIT_22(undefined4 *param_1,undefined4 *param_2)

{
  uint in_EAX;
  
  *param_1 = *param_2;
  out(0xecd2,in_EAX | 0x74);
  _DAT_c2660506502b531b = in_EAX | 0x74;
  return;
}



// ===== _INIT_23 @ 00405930 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_23(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_24 @ 00405950 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_24(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_25 @ 00405970 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_25(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_26 @ 00405c50 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_26(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_27 @ 00405c70 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_27(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_28 @ 00405ce0 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_28(long param_1,uint *param_2,uint *param_3)

{
  int *piVar1;
  char *pcVar2;
  uint uVar3;
  undefined1 uVar4;
  undefined2 in_register_00000002;
  undefined4 in_register_00000004;
  
  if (!SBORROW4((uint)param_3,*(uint *)((long)param_3 + 3))) {
                    /* WARNING: Bad instruction - Truncating control flow here */
    halt_baddata();
  }
  uVar3 = *param_3;
  pcVar2 = (char *)(param_1 * 8 + 0x6d0df40c);
  *pcVar2 = (*pcVar2 + -0x7a) - ((uint)param_3 < *(uint *)((long)param_3 + 3));
  *param_2 = *param_2 & uVar3;
  uVar4 = DAT_1af491c9041c4d63;
  *(undefined8 *)((ulong)uVar3 - 8) = 0xffffffffda10224f;
  piVar1 = (int *)(CONCAT44(in_register_00000004,CONCAT22(in_register_00000002,CONCAT11(0x7a,uVar4))
                           ) + -0x12d63f6a);
  *piVar1 = *piVar1 + -0x4a012e50;
  out(*param_2,(short)(((ulong)param_3 & 0xffffffff) >> 3));
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_29 @ 0042ab60 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_29(undefined1 param_1)

{
  undefined1 *unaff_retaddr;
  
  *unaff_retaddr = param_1;
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_30 @ 0042af50 =====

/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void _INIT_30(uint param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,
             undefined8 param_5,long param_6)

{
  code *pcVar1;
  uint unaff_EBX;
  
  _DAT_ffffffff97716874 = ~_DAT_ffffffff97716874;
  if (unaff_EBX <= param_1) {
    pcVar1 = (code *)swi(1);
    (*pcVar1)();
    return;
  }
  *(undefined8 *)(param_6 + 0x7dba1b33) = param_4;
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_31 @ 0042b060 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_31(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_32 @ 0042b0c0 =====

void _INIT_32(void)

{
  return;
}



// ===== _INIT_33 @ 0042b0e0 =====

void _INIT_33(void)

{
  return;
}



// ===== _INIT_34 @ 0042b2c0 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_34(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_35 @ 0042b2e0 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_35(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_36 @ 0042b330 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_36(void)

{
  uint *puVar1;
  byte in_CL;
  undefined1 unaff_BL;
  undefined1 unaff_BH;
  undefined6 unaff_0000001a;
  
  puVar1 = (uint *)(CONCAT62(unaff_0000001a,CONCAT11(unaff_BH,unaff_BL)) + 0x4b);
  *puVar1 = *puVar1 << (in_CL & 0x1f) | *puVar1 >> 0x20 - (in_CL & 0x1f);
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_37 @ 0042b480 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_37(void)

{
  byte *pbVar1;
  byte in_CL;
  long unaff_RBX;
  
  pbVar1 = &stack0xffffffffa46650ea + unaff_RBX * 4;
  *pbVar1 = *pbVar1 >> (in_CL & 7) | *pbVar1 << 8 - (in_CL & 7);
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_38 @ 0042b4f0 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_38(int param_1,long param_2,undefined8 param_3,undefined8 param_4,long param_5)

{
  int iVar1;
  code *pcVar2;
  undefined4 in_EAX;
  
  iVar1 = *(int *)(param_2 + -0x3ff905ec);
  *(char *)(param_5 + -0x3d) = *(char *)(param_5 + -0x3d) + '\x01';
  pcVar2 = (code *)swi(0x41);
  (*pcVar2)(param_1 + iVar1,in_EAX);
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_39 @ 0042b5e0 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_39(int *param_1)

{
  byte in_AL;
  undefined1 uVar1;
  undefined2 extraout_DX;
  bool in_ZF;
  undefined8 unaff_retaddr;
  
  if (in_ZF) {
    *param_1 = (*param_1 + 0x7ef4d89) - (uint)(in_AL < 0x8c);
    uVar1 = func_0xffffffffe1c30139(param_1,unaff_retaddr);
    out(extraout_DX,uVar1);
                    /* WARNING: Bad instruction - Truncating control flow here */
    halt_baddata();
  }
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_40 @ 0042b640 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_40(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_41 @ 0042b680 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_41(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_42 @ 0042b6e0 =====

/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void _INIT_42(undefined8 param_1,uint param_2)

{
  _DAT_428d7e25 = _DAT_428d7e25 ^ param_2;
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_43 @ 0065b780 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_43(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _INIT_44 @ 0065b7c0 =====

/* WARNING: Control flow encountered bad instruction data */

void _INIT_44(undefined8 param_1,undefined8 param_2,undefined8 param_3,long param_4)

{
  char cVar1;
  undefined8 *puVar2;
  undefined8 *unaff_RBP;
  undefined8 uStack_10;
  
  *(byte *)(param_4 + -0x3e77b584) = *(byte *)(param_4 + -0x3e77b584) & (byte)((ulong)param_3 >> 8);
  puVar2 = (undefined8 *)&stack0xfffffffffffffff8;
  cVar1 = '\b';
  do {
    unaff_RBP = unaff_RBP + -1;
    puVar2 = puVar2 + -1;
    *puVar2 = *unaff_RBP;
    cVar1 = cVar1 + -1;
  } while ('\0' < cVar1);
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== _DT_INIT @ 007e5224 =====

void _DT_INIT(void)

{
  if (PTR___gmon_start___00803480 != (undefined *)0x0) {
    (*(code *)PTR___gmon_start___00803480)();
  }
  return;
}



// ===== _DT_FINI @ 007e5240 =====

void _DT_FINI(void)

{
  return;
}



// ===== __libc_start_main @ 00981000 =====

/* WARNING: Control flow encountered bad instruction data */

void __libc_start_main(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== __gmon_start__ @ 00981008 =====

/* WARNING: Control flow encountered bad instruction data */

void __gmon_start__(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== __cxa_finalize @ 00981020 =====

/* WARNING: Control flow encountered bad instruction data */

void __cxa_finalize(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== fprintf @ 00981028 =====

/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

int fprintf(FILE *__stream,char *__format,...)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== abort @ 00981030 =====

/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void abort(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== __cxa_atexit @ 00981038 =====

/* WARNING: Control flow encountered bad instruction data */

void __cxa_atexit(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== strlen @ 00981040 =====

/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

size_t strlen(char *__s)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== mi_process_info @ 00981048 =====

/* WARNING: Control flow encountered bad instruction data */

void mi_process_info(void)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



// ===== mmap64 @ 00981050 =====

/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void * mmap64(void *__addr,size_t __len,int __prot,int __flags,int __fd,__off64_t __offset)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}



