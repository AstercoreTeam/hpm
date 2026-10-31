                                      1 ;--------------------------------------------------------
                                      2 ; File Created by SDCC : free open source ISO C Compiler
                                      3 ; Version 4.5.0 #15242 (Linux)
                                      4 ;--------------------------------------------------------
                                      5 	.module main
                                      6 	
                                      7 	.optsdcc -mmcs51 --model-small
                                      8 ;--------------------------------------------------------
                                      9 ; Public variables in this module
                                     10 ;--------------------------------------------------------
                                     11 	.globl _main
                                     12 	.globl _hpm_init_all
                                     13 	.globl _servo_start
                                     14 	.globl _servo_set_duty
                                     15 	.globl _app_delay_delay_ms
                                     16 	.globl _CY
                                     17 	.globl _AC
                                     18 	.globl _F0
                                     19 	.globl _RS1
                                     20 	.globl _RS0
                                     21 	.globl _OV
                                     22 	.globl _F1
                                     23 	.globl _P
                                     24 	.globl _SM0
                                     25 	.globl _SM1
                                     26 	.globl _SM2
                                     27 	.globl _REN
                                     28 	.globl _TB8
                                     29 	.globl _RB8
                                     30 	.globl _TI
                                     31 	.globl _RI
                                     32 	.globl _EA
                                     33 	.globl _ES
                                     34 	.globl _ET1
                                     35 	.globl _EX1
                                     36 	.globl _ET0
                                     37 	.globl _EX0
                                     38 	.globl _TF1
                                     39 	.globl _TR1
                                     40 	.globl _TF0
                                     41 	.globl _TR0
                                     42 	.globl _IE1
                                     43 	.globl _IT1
                                     44 	.globl _IE0
                                     45 	.globl _IT0
                                     46 	.globl _P3_7
                                     47 	.globl _P3_6
                                     48 	.globl _P3_5
                                     49 	.globl _P3_4
                                     50 	.globl _P3_3
                                     51 	.globl _P3_2
                                     52 	.globl _P3_1
                                     53 	.globl _P3_0
                                     54 	.globl _P1_7
                                     55 	.globl _P1_6
                                     56 	.globl _P1_5
                                     57 	.globl _P1_4
                                     58 	.globl _P1_3
                                     59 	.globl _P1_2
                                     60 	.globl _P1_1
                                     61 	.globl _P1_0
                                     62 	.globl _WDT_CONTR
                                     63 	.globl _AUXR
                                     64 	.globl _B
                                     65 	.globl _ACC
                                     66 	.globl _PSW
                                     67 	.globl _IP
                                     68 	.globl _IE
                                     69 	.globl _SBUF
                                     70 	.globl _SCON
                                     71 	.globl _TH1
                                     72 	.globl _TH0
                                     73 	.globl _TL1
                                     74 	.globl _TL0
                                     75 	.globl _TMOD
                                     76 	.globl _TCON
                                     77 	.globl _PCON
                                     78 	.globl _DPH
                                     79 	.globl _DPL
                                     80 	.globl _SP
                                     81 	.globl _P3
                                     82 	.globl _P2
                                     83 	.globl _P1
                                     84 	.globl _P0
                                     85 ;--------------------------------------------------------
                                     86 ; special function registers
                                     87 ;--------------------------------------------------------
                                     88 	.area RSEG    (ABS,DATA)
      000000                         89 	.org 0x0000
                           000080    90 _P0	=	0x0080
                           000090    91 _P1	=	0x0090
                           0000A0    92 _P2	=	0x00a0
                           0000B0    93 _P3	=	0x00b0
                           000081    94 _SP	=	0x0081
                           000082    95 _DPL	=	0x0082
                           000083    96 _DPH	=	0x0083
                           000087    97 _PCON	=	0x0087
                           000088    98 _TCON	=	0x0088
                           000089    99 _TMOD	=	0x0089
                           00008A   100 _TL0	=	0x008a
                           00008B   101 _TL1	=	0x008b
                           00008C   102 _TH0	=	0x008c
                           00008D   103 _TH1	=	0x008d
                           000098   104 _SCON	=	0x0098
                           000099   105 _SBUF	=	0x0099
                           0000A8   106 _IE	=	0x00a8
                           0000B8   107 _IP	=	0x00b8
                           0000D0   108 _PSW	=	0x00d0
                           0000E0   109 _ACC	=	0x00e0
                           0000F0   110 _B	=	0x00f0
                           00008E   111 _AUXR	=	0x008e
                           0000C1   112 _WDT_CONTR	=	0x00c1
                                    113 ;--------------------------------------------------------
                                    114 ; special function bits
                                    115 ;--------------------------------------------------------
                                    116 	.area RSEG    (ABS,DATA)
      000000                        117 	.org 0x0000
                           000090   118 _P1_0	=	0x0090
                           000091   119 _P1_1	=	0x0091
                           000092   120 _P1_2	=	0x0092
                           000093   121 _P1_3	=	0x0093
                           000094   122 _P1_4	=	0x0094
                           000095   123 _P1_5	=	0x0095
                           000096   124 _P1_6	=	0x0096
                           000097   125 _P1_7	=	0x0097
                           0000B0   126 _P3_0	=	0x00b0
                           0000B1   127 _P3_1	=	0x00b1
                           0000B2   128 _P3_2	=	0x00b2
                           0000B3   129 _P3_3	=	0x00b3
                           0000B4   130 _P3_4	=	0x00b4
                           0000B5   131 _P3_5	=	0x00b5
                           0000B6   132 _P3_6	=	0x00b6
                           0000B7   133 _P3_7	=	0x00b7
                           000088   134 _IT0	=	0x0088
                           000089   135 _IE0	=	0x0089
                           00008A   136 _IT1	=	0x008a
                           00008B   137 _IE1	=	0x008b
                           00008C   138 _TR0	=	0x008c
                           00008D   139 _TF0	=	0x008d
                           00008E   140 _TR1	=	0x008e
                           00008F   141 _TF1	=	0x008f
                           0000A8   142 _EX0	=	0x00a8
                           0000A9   143 _ET0	=	0x00a9
                           0000AA   144 _EX1	=	0x00aa
                           0000AB   145 _ET1	=	0x00ab
                           0000AC   146 _ES	=	0x00ac
                           0000AF   147 _EA	=	0x00af
                           000098   148 _RI	=	0x0098
                           000099   149 _TI	=	0x0099
                           00009A   150 _RB8	=	0x009a
                           00009B   151 _TB8	=	0x009b
                           00009C   152 _REN	=	0x009c
                           00009D   153 _SM2	=	0x009d
                           00009E   154 _SM1	=	0x009e
                           00009F   155 _SM0	=	0x009f
                           0000D0   156 _P	=	0x00d0
                           0000D1   157 _F1	=	0x00d1
                           0000D2   158 _OV	=	0x00d2
                           0000D3   159 _RS0	=	0x00d3
                           0000D4   160 _RS1	=	0x00d4
                           0000D5   161 _F0	=	0x00d5
                           0000D6   162 _AC	=	0x00d6
                           0000D7   163 _CY	=	0x00d7
                                    164 ;--------------------------------------------------------
                                    165 ; overlayable register banks
                                    166 ;--------------------------------------------------------
                                    167 	.area REG_BANK_0	(REL,OVR,DATA)
      000000                        168 	.ds 8
                                    169 ;--------------------------------------------------------
                                    170 ; internal ram data
                                    171 ;--------------------------------------------------------
                                    172 	.area DSEG    (DATA)
      00000F                        173 _main_angles_10000_17:
      00000F                        174 	.ds 10
                                    175 ;--------------------------------------------------------
                                    176 ; overlayable items in internal ram
                                    177 ;--------------------------------------------------------
                                    178 ;--------------------------------------------------------
                                    179 ; Stack segment in internal ram
                                    180 ;--------------------------------------------------------
                                    181 	.area SSEG
      000021                        182 __start__stack:
      000021                        183 	.ds	1
                                    184 
                                    185 ;--------------------------------------------------------
                                    186 ; indirectly addressable internal ram data
                                    187 ;--------------------------------------------------------
                                    188 	.area ISEG    (DATA)
                                    189 ;--------------------------------------------------------
                                    190 ; absolute internal ram data
                                    191 ;--------------------------------------------------------
                                    192 	.area IABS    (ABS,DATA)
                                    193 	.area IABS    (ABS,DATA)
                                    194 ;--------------------------------------------------------
                                    195 ; bit data
                                    196 ;--------------------------------------------------------
                                    197 	.area BSEG    (BIT)
                                    198 ;--------------------------------------------------------
                                    199 ; paged external ram data
                                    200 ;--------------------------------------------------------
                                    201 	.area PSEG    (PAG,XDATA)
                                    202 ;--------------------------------------------------------
                                    203 ; uninitialized external ram data
                                    204 ;--------------------------------------------------------
                                    205 	.area XSEG    (XDATA)
                                    206 ;--------------------------------------------------------
                                    207 ; absolute external ram data
                                    208 ;--------------------------------------------------------
                                    209 	.area XABS    (ABS,XDATA)
                                    210 ;--------------------------------------------------------
                                    211 ; initialized external ram data
                                    212 ;--------------------------------------------------------
                                    213 	.area XISEG   (XDATA)
                                    214 	.area HOME    (CODE)
                                    215 	.area GSINIT0 (CODE)
                                    216 	.area GSINIT1 (CODE)
                                    217 	.area GSINIT2 (CODE)
                                    218 	.area GSINIT3 (CODE)
                                    219 	.area GSINIT4 (CODE)
                                    220 	.area GSINIT5 (CODE)
                                    221 	.area GSINIT  (CODE)
                                    222 	.area GSFINAL (CODE)
                                    223 	.area CSEG    (CODE)
                                    224 ;--------------------------------------------------------
                                    225 ; interrupt vector
                                    226 ;--------------------------------------------------------
                                    227 	.area HOME    (CODE)
      000000                        228 __interrupt_vect:
      000000 02 00 90         [24]  229 	ljmp	__sdcc_gsinit_startup
      000003 32               [24]  230 	reti
      000004                        231 	.ds	7
      00000B 02 01 A7         [24]  232 	ljmp	_pwm_timer0_isr
      00000E                        233 	.ds	5
      000013 32               [24]  234 	reti
      000014                        235 	.ds	7
      00001B 02 01 04         [24]  236 	ljmp	_time_timer1_isr
                                    237 ; restartable atomic support routines
      00001E                        238 	.ds	2
      000020                        239 sdcc_atomic_exchange_rollback_start::
      000020 00               [12]  240 	nop
      000021 00               [12]  241 	nop
      000022                        242 sdcc_atomic_exchange_pdata_impl:
      000022 E2               [24]  243 	movx	a, @r0
      000023 FB               [12]  244 	mov	r3, a
      000024 EA               [12]  245 	mov	a, r2
      000025 F2               [24]  246 	movx	@r0, a
      000026 80 2C            [24]  247 	sjmp	sdcc_atomic_exchange_exit
      000028 00               [12]  248 	nop
      000029 00               [12]  249 	nop
      00002A                        250 sdcc_atomic_exchange_xdata_impl:
      00002A E0               [24]  251 	movx	a, @dptr
      00002B FB               [12]  252 	mov	r3, a
      00002C EA               [12]  253 	mov	a, r2
      00002D F0               [24]  254 	movx	@dptr, a
      00002E 80 24            [24]  255 	sjmp	sdcc_atomic_exchange_exit
      000030                        256 sdcc_atomic_compare_exchange_idata_impl:
      000030 E6               [12]  257 	mov	a, @r0
      000031 B5 02 02         [24]  258 	cjne	a, ar2, .+#5
      000034 EB               [12]  259 	mov	a, r3
      000035 F6               [12]  260 	mov	@r0, a
      000036 22               [24]  261 	ret
      000037 00               [12]  262 	nop
      000038                        263 sdcc_atomic_compare_exchange_pdata_impl:
      000038 E2               [24]  264 	movx	a, @r0
      000039 B5 02 02         [24]  265 	cjne	a, ar2, .+#5
      00003C EB               [12]  266 	mov	a, r3
      00003D F2               [24]  267 	movx	@r0, a
      00003E 22               [24]  268 	ret
      00003F 00               [12]  269 	nop
      000040                        270 sdcc_atomic_compare_exchange_xdata_impl:
      000040 E0               [24]  271 	movx	a, @dptr
      000041 B5 02 02         [24]  272 	cjne	a, ar2, .+#5
      000044 EB               [12]  273 	mov	a, r3
      000045 F0               [24]  274 	movx	@dptr, a
      000046 22               [24]  275 	ret
      000047                        276 sdcc_atomic_exchange_rollback_end::
                                    277 
      000047                        278 sdcc_atomic_exchange_gptr_impl::
      000047 30 F6 E0         [24]  279 	jnb	b.6, sdcc_atomic_exchange_xdata_impl
      00004A A8 82            [24]  280 	mov	r0, dpl
      00004C 20 F5 D3         [24]  281 	jb	b.5, sdcc_atomic_exchange_pdata_impl
      00004F                        282 sdcc_atomic_exchange_idata_impl:
      00004F EA               [12]  283 	mov	a, r2
      000050 C6               [12]  284 	xch	a, @r0
      000051 F5 82            [12]  285 	mov	dpl, a
      000053 22               [24]  286 	ret
      000054                        287 sdcc_atomic_exchange_exit:
      000054 8B 82            [24]  288 	mov	dpl, r3
      000056 22               [24]  289 	ret
      000057                        290 sdcc_atomic_compare_exchange_gptr_impl::
      000057 30 F6 E6         [24]  291 	jnb	b.6, sdcc_atomic_compare_exchange_xdata_impl
      00005A A8 82            [24]  292 	mov	r0, dpl
      00005C 20 F5 D9         [24]  293 	jb	b.5, sdcc_atomic_compare_exchange_pdata_impl
      00005F 80 CF            [24]  294 	sjmp	sdcc_atomic_compare_exchange_idata_impl
                                    295 ;--------------------------------------------------------
                                    296 ; global & static initialisations
                                    297 ;--------------------------------------------------------
                                    298 	.area HOME    (CODE)
                                    299 	.area GSINIT  (CODE)
                                    300 	.area GSFINAL (CODE)
                                    301 	.area GSINIT  (CODE)
                                    302 	.globl __sdcc_gsinit_startup
                                    303 	.globl __sdcc_program_startup
                                    304 	.globl __start__stack
                                    305 	.globl __mcs51_genXINIT
                                    306 	.globl __mcs51_genXRAMCLEAR
                                    307 	.globl __mcs51_genRAMCLEAR
                                    308 	.area GSFINAL (CODE)
      0000FB 02 00 61         [24]  309 	ljmp	__sdcc_program_startup
                                    310 ;--------------------------------------------------------
                                    311 ; Home
                                    312 ;--------------------------------------------------------
                                    313 	.area HOME    (CODE)
                                    314 	.area HOME    (CODE)
      000061                        315 __sdcc_program_startup:
      000061 02 04 78         [24]  316 	ljmp	_main
                                    317 ;	return from main will return to caller
                                    318 ;--------------------------------------------------------
                                    319 ; code
                                    320 ;--------------------------------------------------------
                                    321 	.area CSEG    (CODE)
                                    322 ;------------------------------------------------------------
                                    323 ;Allocation info for local variables in function 'angle_to_permille'
                                    324 ;------------------------------------------------------------
                                    325 ;deg           Allocated to registers r6 r7 
                                    326 ;------------------------------------------------------------
                                    327 ;	../main.c:4: static unsigned int angle_to_permille(int deg) {
                                    328 ;	-----------------------------------------
                                    329 ;	 function angle_to_permille
                                    330 ;	-----------------------------------------
      000437                        331 _angle_to_permille:
                           000007   332 	ar7 = 0x07
                           000006   333 	ar6 = 0x06
                           000005   334 	ar5 = 0x05
                           000004   335 	ar4 = 0x04
                           000003   336 	ar3 = 0x03
                           000002   337 	ar2 = 0x02
                           000001   338 	ar1 = 0x01
                           000000   339 	ar0 = 0x00
      000437 AE 82            [24]  340 	mov	r6, dpl
      000439 AF 83            [24]  341 	mov	r7, dph
                                    342 ;	../main.c:5: if (deg > 120) deg = 120;
      00043B C3               [12]  343 	clr	c
      00043C 74 78            [12]  344 	mov	a,#0x78
      00043E 9E               [12]  345 	subb	a,r6
      00043F 74 80            [12]  346 	mov	a,#(0x00 ^ 0x80)
      000441 8F F0            [24]  347 	mov	b,r7
      000443 63 F0 80         [24]  348 	xrl	b,#0x80
      000446 95 F0            [12]  349 	subb	a,b
      000448 50 04            [24]  350 	jnc	00102$
      00044A 7E 78            [12]  351 	mov	r6,#0x78
      00044C 7F 00            [12]  352 	mov	r7,#0x00
      00044E                        353 00102$:
                                    354 ;	../main.c:6: if (deg < 0)   deg = 0;
      00044E EF               [12]  355 	mov	a,r7
      00044F 30 E7 04         [24]  356 	jnb	acc.7,00104$
      000452 7E 00            [12]  357 	mov	r6,#0x00
      000454 7F 00            [12]  358 	mov	r7,#0x00
      000456                        359 00104$:
                                    360 ;	../main.c:7: return 165 - ((deg * 115) / 120);
      000456 8E 19            [24]  361 	mov	__mulint_PARM_2,r6
      000458 8F 1A            [24]  362 	mov	(__mulint_PARM_2 + 1),r7
      00045A 90 00 73         [24]  363 	mov	dptr,#0x0073
      00045D 12 04 EA         [24]  364 	lcall	__mulint
      000460 75 19 78         [24]  365 	mov	__divsint_PARM_2,#0x78
      000463 75 1A 00         [24]  366 	mov	(__divsint_PARM_2 + 1),#0x00
      000466 12 05 07         [24]  367 	lcall	__divsint
      000469 AE 82            [24]  368 	mov	r6, dpl
      00046B AF 83            [24]  369 	mov	r7, dph
      00046D 74 A5            [12]  370 	mov	a,#0xa5
      00046F C3               [12]  371 	clr	c
      000470 9E               [12]  372 	subb	a,r6
      000471 F5 82            [12]  373 	mov	dpl,a
      000473 E4               [12]  374 	clr	a
      000474 9F               [12]  375 	subb	a,r7
      000475 F5 83            [12]  376 	mov	dph,a
                                    377 ;	../main.c:8: }
      000477 22               [24]  378 	ret
                                    379 ;------------------------------------------------------------
                                    380 ;Allocation info for local variables in function 'main'
                                    381 ;------------------------------------------------------------
                                    382 ;i             Allocated to registers r7 
                                    383 ;angles        Allocated with name '_main_angles_10000_17'
                                    384 ;------------------------------------------------------------
                                    385 ;	../main.c:10: int main(void) {
                                    386 ;	-----------------------------------------
                                    387 ;	 function main
                                    388 ;	-----------------------------------------
      000478                        389 _main:
                                    390 ;	../main.c:11: unsigned char i = 0;
                                    391 ;	../main.c:12: const int angles[5] = { 0, 30, 60, 90, 120 };
      000478 E4               [12]  392 	clr	a
      000479 FF               [12]  393 	mov	r7,a
      00047A F5 0F            [12]  394 	mov	(_main_angles_10000_17 + 0),a
      00047C F5 10            [12]  395 	mov	(_main_angles_10000_17 + 1),a
      00047E 75 11 1E         [24]  396 	mov	((_main_angles_10000_17 + 0x0002) + 0),#0x1e
      000481 F5 12            [12]  397 	mov	((_main_angles_10000_17 + 0x0002) + 1),a
      000483 75 13 3C         [24]  398 	mov	((_main_angles_10000_17 + 0x0004) + 0),#0x3c
      000486 F5 14            [12]  399 	mov	((_main_angles_10000_17 + 0x0004) + 1),a
      000488 75 15 5A         [24]  400 	mov	((_main_angles_10000_17 + 0x0006) + 0),#0x5a
      00048B F5 16            [12]  401 	mov	((_main_angles_10000_17 + 0x0006) + 1),a
      00048D 75 17 78         [24]  402 	mov	((_main_angles_10000_17 + 0x0008) + 0),#0x78
      000490 F5 18            [12]  403 	mov	((_main_angles_10000_17 + 0x0008) + 1),a
                                    404 ;	../main.c:14: hpm_init_all();
      000492 C0 07            [24]  405 	push	ar7
      000494 12 00 FE         [24]  406 	lcall	_hpm_init_all
                                    407 ;	../main.c:15: servo_start();
      000497 12 02 87         [24]  408 	lcall	_servo_start
      00049A D0 07            [24]  409 	pop	ar7
                                    410 ;	../main.c:17: while (1) {
      00049C                        411 00104$:
                                    412 ;	../main.c:18: servo_set_duty(angle_to_permille(angles[i]));
      00049C EF               [12]  413 	mov	a,r7
      00049D 2F               [12]  414 	add	a,r7
      00049E 24 0F            [12]  415 	add	a, #_main_angles_10000_17
      0004A0 F9               [12]  416 	mov	r1,a
      0004A1 87 82            [24]  417 	mov	dpl,@r1
      0004A3 09               [12]  418 	inc	r1
      0004A4 87 83            [24]  419 	mov	dph,@r1
      0004A6 19               [12]  420 	dec	r1
      0004A7 C0 07            [24]  421 	push	ar7
      0004A9 12 04 37         [24]  422 	lcall	_angle_to_permille
      0004AC 12 02 BE         [24]  423 	lcall	_servo_set_duty
                                    424 ;	../main.c:21: app_delay_delay_ms(1500);
      0004AF 90 05 DC         [24]  425 	mov	dptr,#0x05dc
      0004B2 12 01 8C         [24]  426 	lcall	_app_delay_delay_ms
      0004B5 D0 07            [24]  427 	pop	ar7
                                    428 ;	../main.c:23: i++;
      0004B7 0F               [12]  429 	inc	r7
                                    430 ;	../main.c:24: if (i >= 5) i = 0;
      0004B8 BF 05 00         [24]  431 	cjne	r7,#0x05,00120$
      0004BB                        432 00120$:
      0004BB 40 DF            [24]  433 	jc	00104$
      0004BD 7F 00            [12]  434 	mov	r7,#0x00
                                    435 ;	../main.c:26: return 0;
                                    436 ;	../main.c:27: }
      0004BF 80 DB            [24]  437 	sjmp	00104$
                                    438 	.area CSEG    (CODE)
                                    439 	.area CONST   (CODE)
                                    440 	.area XINIT   (CODE)
                                    441 	.area CABS    (ABS,CODE)
