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
                                     12 	.globl _serial_read
                                     13 	.globl _serial_available
                                     14 	.globl _serial_write
                                     15 	.globl _serial_init
                                     16 	.globl _hpm_pin_P1_1_write
                                     17 	.globl _hpm_pin_P1_1_init
                                     18 	.globl _CDC_flush
                                     19 	.globl _ch552g_init
                                     20 	.globl _UIF_BUS_RST
                                     21 	.globl _UIF_DETECT
                                     22 	.globl _UIF_TRANSFER
                                     23 	.globl _UIF_SUSPEND
                                     24 	.globl _UIF_HST_SOF
                                     25 	.globl _UIF_FIFO_OV
                                     26 	.globl _U_SIE_FREE
                                     27 	.globl _U_TOG_OK
                                     28 	.globl _U_IS_NAK
                                     29 	.globl _ADC_CHAN0
                                     30 	.globl _ADC_CHAN1
                                     31 	.globl _CMP_CHAN
                                     32 	.globl _ADC_START
                                     33 	.globl _ADC_IF
                                     34 	.globl _CMP_IF
                                     35 	.globl _CMPO
                                     36 	.globl _U1RI
                                     37 	.globl _U1TI
                                     38 	.globl _U1RB8
                                     39 	.globl _U1TB8
                                     40 	.globl _U1REN
                                     41 	.globl _U1SMOD
                                     42 	.globl _U1SM0
                                     43 	.globl _S0_R_FIFO
                                     44 	.globl _S0_T_FIFO
                                     45 	.globl _S0_FREE
                                     46 	.globl _S0_IF_BYTE
                                     47 	.globl _S0_IF_FIRST
                                     48 	.globl _S0_IF_OV
                                     49 	.globl _S0_FST_ACT
                                     50 	.globl _CP_RL2
                                     51 	.globl _C_T2
                                     52 	.globl _TR2
                                     53 	.globl _EXEN2
                                     54 	.globl _TCLK
                                     55 	.globl _RCLK
                                     56 	.globl _EXF2
                                     57 	.globl _CAP1F
                                     58 	.globl _TF2
                                     59 	.globl _RI
                                     60 	.globl _TI
                                     61 	.globl _RB8
                                     62 	.globl _TB8
                                     63 	.globl _REN
                                     64 	.globl _SM2
                                     65 	.globl _SM1
                                     66 	.globl _SM0
                                     67 	.globl _IT0
                                     68 	.globl _IE0
                                     69 	.globl _IT1
                                     70 	.globl _IE1
                                     71 	.globl _TR0
                                     72 	.globl _TF0
                                     73 	.globl _TR1
                                     74 	.globl _TF1
                                     75 	.globl _RXD
                                     76 	.globl _PWM1_
                                     77 	.globl _TXD
                                     78 	.globl _PWM2_
                                     79 	.globl _AIN3
                                     80 	.globl _VBUS1
                                     81 	.globl _INT0
                                     82 	.globl _TXD1_
                                     83 	.globl _INT1
                                     84 	.globl _T0
                                     85 	.globl _RXD1_
                                     86 	.globl _PWM2
                                     87 	.globl _T1
                                     88 	.globl _UDP
                                     89 	.globl _UDM
                                     90 	.globl _TIN0
                                     91 	.globl _CAP1
                                     92 	.globl _T2
                                     93 	.globl _AIN0
                                     94 	.globl _VBUS2
                                     95 	.globl _TIN1
                                     96 	.globl _CAP2
                                     97 	.globl _T2EX
                                     98 	.globl _RXD_
                                     99 	.globl _TXD_
                                    100 	.globl _AIN1
                                    101 	.globl _UCC1
                                    102 	.globl _TIN2
                                    103 	.globl _SCS
                                    104 	.globl _CAP1_
                                    105 	.globl _T2_
                                    106 	.globl _AIN2
                                    107 	.globl _UCC2
                                    108 	.globl _TIN3
                                    109 	.globl _PWM1
                                    110 	.globl _MOSI
                                    111 	.globl _TIN4
                                    112 	.globl _RXD1
                                    113 	.globl _MISO
                                    114 	.globl _TIN5
                                    115 	.globl _TXD1
                                    116 	.globl _SCK
                                    117 	.globl _IE_SPI0
                                    118 	.globl _IE_TKEY
                                    119 	.globl _IE_USB
                                    120 	.globl _IE_ADC
                                    121 	.globl _IE_UART1
                                    122 	.globl _IE_PWMX
                                    123 	.globl _IE_GPIO
                                    124 	.globl _IE_WDOG
                                    125 	.globl _PX0
                                    126 	.globl _PT0
                                    127 	.globl _PX1
                                    128 	.globl _PT1
                                    129 	.globl _PS
                                    130 	.globl _PT2
                                    131 	.globl _PL_FLAG
                                    132 	.globl _PH_FLAG
                                    133 	.globl _EX0
                                    134 	.globl _ET0
                                    135 	.globl _EX1
                                    136 	.globl _ET1
                                    137 	.globl _ES
                                    138 	.globl _ET2
                                    139 	.globl _E_DIS
                                    140 	.globl _EA
                                    141 	.globl _P
                                    142 	.globl _F1
                                    143 	.globl _OV
                                    144 	.globl _RS0
                                    145 	.globl _RS1
                                    146 	.globl _F0
                                    147 	.globl _AC
                                    148 	.globl _CY
                                    149 	.globl _UEP1_DMA_H
                                    150 	.globl _UEP1_DMA_L
                                    151 	.globl _UEP1_DMA
                                    152 	.globl _UEP0_DMA_H
                                    153 	.globl _UEP0_DMA_L
                                    154 	.globl _UEP0_DMA
                                    155 	.globl _UEP2_3_MOD
                                    156 	.globl _UEP4_1_MOD
                                    157 	.globl _UEP3_DMA_H
                                    158 	.globl _UEP3_DMA_L
                                    159 	.globl _UEP3_DMA
                                    160 	.globl _UEP2_DMA_H
                                    161 	.globl _UEP2_DMA_L
                                    162 	.globl _UEP2_DMA
                                    163 	.globl _USB_DEV_AD
                                    164 	.globl _USB_CTRL
                                    165 	.globl _USB_INT_EN
                                    166 	.globl _UEP4_T_LEN
                                    167 	.globl _UEP4_CTRL
                                    168 	.globl _UEP0_T_LEN
                                    169 	.globl _UEP0_CTRL
                                    170 	.globl _USB_RX_LEN
                                    171 	.globl _USB_MIS_ST
                                    172 	.globl _USB_INT_ST
                                    173 	.globl _USB_INT_FG
                                    174 	.globl _UEP3_T_LEN
                                    175 	.globl _UEP3_CTRL
                                    176 	.globl _UEP2_T_LEN
                                    177 	.globl _UEP2_CTRL
                                    178 	.globl _UEP1_T_LEN
                                    179 	.globl _UEP1_CTRL
                                    180 	.globl _UDEV_CTRL
                                    181 	.globl _USB_C_CTRL
                                    182 	.globl _TKEY_DATH
                                    183 	.globl _TKEY_DATL
                                    184 	.globl _TKEY_DAT
                                    185 	.globl _TKEY_CTRL
                                    186 	.globl _ADC_DATA
                                    187 	.globl _ADC_CFG
                                    188 	.globl _ADC_CTRL
                                    189 	.globl _SBAUD1
                                    190 	.globl _SBUF1
                                    191 	.globl _SCON1
                                    192 	.globl _SPI0_SETUP
                                    193 	.globl _SPI0_CK_SE
                                    194 	.globl _SPI0_CTRL
                                    195 	.globl _SPI0_DATA
                                    196 	.globl _SPI0_STAT
                                    197 	.globl _PWM_CK_SE
                                    198 	.globl _PWM_CTRL
                                    199 	.globl _PWM_DATA1
                                    200 	.globl _PWM_DATA2
                                    201 	.globl _T2CAP1H
                                    202 	.globl _T2CAP1L
                                    203 	.globl _T2CAP1
                                    204 	.globl _TH2
                                    205 	.globl _TL2
                                    206 	.globl _T2COUNT
                                    207 	.globl _RCAP2H
                                    208 	.globl _RCAP2L
                                    209 	.globl _RCAP2
                                    210 	.globl _T2MOD
                                    211 	.globl _T2CON
                                    212 	.globl _SBUF
                                    213 	.globl _SCON
                                    214 	.globl _TH1
                                    215 	.globl _TH0
                                    216 	.globl _TL1
                                    217 	.globl _TL0
                                    218 	.globl _TMOD
                                    219 	.globl _TCON
                                    220 	.globl _XBUS_AUX
                                    221 	.globl _PIN_FUNC
                                    222 	.globl _P3_DIR_PU
                                    223 	.globl _P3_MOD_OC
                                    224 	.globl _P3
                                    225 	.globl _P2
                                    226 	.globl _P1_DIR_PU
                                    227 	.globl _P1_MOD_OC
                                    228 	.globl _P1
                                    229 	.globl _ROM_CTRL
                                    230 	.globl _ROM_DATA_H
                                    231 	.globl _ROM_DATA_L
                                    232 	.globl _ROM_DATA
                                    233 	.globl _ROM_ADDR_H
                                    234 	.globl _ROM_ADDR_L
                                    235 	.globl _ROM_ADDR
                                    236 	.globl _GPIO_IE
                                    237 	.globl _IP_EX
                                    238 	.globl _IE_EX
                                    239 	.globl _IP
                                    240 	.globl _IE
                                    241 	.globl _WDOG_COUNT
                                    242 	.globl _RESET_KEEP
                                    243 	.globl _WAKE_CTRL
                                    244 	.globl _CLOCK_CFG
                                    245 	.globl _PCON
                                    246 	.globl _GLOBAL_CFG
                                    247 	.globl _SAFE_MOD
                                    248 	.globl _DPH
                                    249 	.globl _DPL
                                    250 	.globl _SP
                                    251 	.globl _B
                                    252 	.globl _ACC
                                    253 	.globl _PSW
                                    254 ;--------------------------------------------------------
                                    255 ; special function registers
                                    256 ;--------------------------------------------------------
                                    257 	.area RSEG    (ABS,DATA)
      000000                        258 	.org 0x0000
                           0000D0   259 _PSW	=	0x00d0
                           0000E0   260 _ACC	=	0x00e0
                           0000F0   261 _B	=	0x00f0
                           000081   262 _SP	=	0x0081
                           000082   263 _DPL	=	0x0082
                           000083   264 _DPH	=	0x0083
                           0000A1   265 _SAFE_MOD	=	0x00a1
                           0000B1   266 _GLOBAL_CFG	=	0x00b1
                           000087   267 _PCON	=	0x0087
                           0000B9   268 _CLOCK_CFG	=	0x00b9
                           0000A9   269 _WAKE_CTRL	=	0x00a9
                           0000FE   270 _RESET_KEEP	=	0x00fe
                           0000FF   271 _WDOG_COUNT	=	0x00ff
                           0000A8   272 _IE	=	0x00a8
                           0000B8   273 _IP	=	0x00b8
                           0000E8   274 _IE_EX	=	0x00e8
                           0000E9   275 _IP_EX	=	0x00e9
                           0000C7   276 _GPIO_IE	=	0x00c7
                           008584   277 _ROM_ADDR	=	0x8584
                           000084   278 _ROM_ADDR_L	=	0x0084
                           000085   279 _ROM_ADDR_H	=	0x0085
                           008F8E   280 _ROM_DATA	=	0x8f8e
                           00008E   281 _ROM_DATA_L	=	0x008e
                           00008F   282 _ROM_DATA_H	=	0x008f
                           000086   283 _ROM_CTRL	=	0x0086
                           000090   284 _P1	=	0x0090
                           000092   285 _P1_MOD_OC	=	0x0092
                           000093   286 _P1_DIR_PU	=	0x0093
                           0000A0   287 _P2	=	0x00a0
                           0000B0   288 _P3	=	0x00b0
                           000096   289 _P3_MOD_OC	=	0x0096
                           000097   290 _P3_DIR_PU	=	0x0097
                           0000C6   291 _PIN_FUNC	=	0x00c6
                           0000A2   292 _XBUS_AUX	=	0x00a2
                           000088   293 _TCON	=	0x0088
                           000089   294 _TMOD	=	0x0089
                           00008A   295 _TL0	=	0x008a
                           00008B   296 _TL1	=	0x008b
                           00008C   297 _TH0	=	0x008c
                           00008D   298 _TH1	=	0x008d
                           000098   299 _SCON	=	0x0098
                           000099   300 _SBUF	=	0x0099
                           0000C8   301 _T2CON	=	0x00c8
                           0000C9   302 _T2MOD	=	0x00c9
                           00CBCA   303 _RCAP2	=	0xcbca
                           0000CA   304 _RCAP2L	=	0x00ca
                           0000CB   305 _RCAP2H	=	0x00cb
                           00CDCC   306 _T2COUNT	=	0xcdcc
                           0000CC   307 _TL2	=	0x00cc
                           0000CD   308 _TH2	=	0x00cd
                           00CFCE   309 _T2CAP1	=	0xcfce
                           0000CE   310 _T2CAP1L	=	0x00ce
                           0000CF   311 _T2CAP1H	=	0x00cf
                           00009B   312 _PWM_DATA2	=	0x009b
                           00009C   313 _PWM_DATA1	=	0x009c
                           00009D   314 _PWM_CTRL	=	0x009d
                           00009E   315 _PWM_CK_SE	=	0x009e
                           0000F8   316 _SPI0_STAT	=	0x00f8
                           0000F9   317 _SPI0_DATA	=	0x00f9
                           0000FA   318 _SPI0_CTRL	=	0x00fa
                           0000FB   319 _SPI0_CK_SE	=	0x00fb
                           0000FC   320 _SPI0_SETUP	=	0x00fc
                           0000C0   321 _SCON1	=	0x00c0
                           0000C1   322 _SBUF1	=	0x00c1
                           0000C2   323 _SBAUD1	=	0x00c2
                           000080   324 _ADC_CTRL	=	0x0080
                           00009A   325 _ADC_CFG	=	0x009a
                           00009F   326 _ADC_DATA	=	0x009f
                           0000C3   327 _TKEY_CTRL	=	0x00c3
                           00C5C4   328 _TKEY_DAT	=	0xc5c4
                           0000C4   329 _TKEY_DATL	=	0x00c4
                           0000C5   330 _TKEY_DATH	=	0x00c5
                           000091   331 _USB_C_CTRL	=	0x0091
                           0000D1   332 _UDEV_CTRL	=	0x00d1
                           0000D2   333 _UEP1_CTRL	=	0x00d2
                           0000D3   334 _UEP1_T_LEN	=	0x00d3
                           0000D4   335 _UEP2_CTRL	=	0x00d4
                           0000D5   336 _UEP2_T_LEN	=	0x00d5
                           0000D6   337 _UEP3_CTRL	=	0x00d6
                           0000D7   338 _UEP3_T_LEN	=	0x00d7
                           0000D8   339 _USB_INT_FG	=	0x00d8
                           0000D9   340 _USB_INT_ST	=	0x00d9
                           0000DA   341 _USB_MIS_ST	=	0x00da
                           0000DB   342 _USB_RX_LEN	=	0x00db
                           0000DC   343 _UEP0_CTRL	=	0x00dc
                           0000DD   344 _UEP0_T_LEN	=	0x00dd
                           0000DE   345 _UEP4_CTRL	=	0x00de
                           0000DF   346 _UEP4_T_LEN	=	0x00df
                           0000E1   347 _USB_INT_EN	=	0x00e1
                           0000E2   348 _USB_CTRL	=	0x00e2
                           0000E3   349 _USB_DEV_AD	=	0x00e3
                           00E5E4   350 _UEP2_DMA	=	0xe5e4
                           0000E4   351 _UEP2_DMA_L	=	0x00e4
                           0000E5   352 _UEP2_DMA_H	=	0x00e5
                           00E7E6   353 _UEP3_DMA	=	0xe7e6
                           0000E6   354 _UEP3_DMA_L	=	0x00e6
                           0000E7   355 _UEP3_DMA_H	=	0x00e7
                           0000EA   356 _UEP4_1_MOD	=	0x00ea
                           0000EB   357 _UEP2_3_MOD	=	0x00eb
                           00EDEC   358 _UEP0_DMA	=	0xedec
                           0000EC   359 _UEP0_DMA_L	=	0x00ec
                           0000ED   360 _UEP0_DMA_H	=	0x00ed
                           00EFEE   361 _UEP1_DMA	=	0xefee
                           0000EE   362 _UEP1_DMA_L	=	0x00ee
                           0000EF   363 _UEP1_DMA_H	=	0x00ef
                                    364 ;--------------------------------------------------------
                                    365 ; special function bits
                                    366 ;--------------------------------------------------------
                                    367 	.area RSEG    (ABS,DATA)
      000000                        368 	.org 0x0000
                           0000D7   369 _CY	=	0x00d7
                           0000D6   370 _AC	=	0x00d6
                           0000D5   371 _F0	=	0x00d5
                           0000D4   372 _RS1	=	0x00d4
                           0000D3   373 _RS0	=	0x00d3
                           0000D2   374 _OV	=	0x00d2
                           0000D1   375 _F1	=	0x00d1
                           0000D0   376 _P	=	0x00d0
                           0000AF   377 _EA	=	0x00af
                           0000AE   378 _E_DIS	=	0x00ae
                           0000AD   379 _ET2	=	0x00ad
                           0000AC   380 _ES	=	0x00ac
                           0000AB   381 _ET1	=	0x00ab
                           0000AA   382 _EX1	=	0x00aa
                           0000A9   383 _ET0	=	0x00a9
                           0000A8   384 _EX0	=	0x00a8
                           0000BF   385 _PH_FLAG	=	0x00bf
                           0000BE   386 _PL_FLAG	=	0x00be
                           0000BD   387 _PT2	=	0x00bd
                           0000BC   388 _PS	=	0x00bc
                           0000BB   389 _PT1	=	0x00bb
                           0000BA   390 _PX1	=	0x00ba
                           0000B9   391 _PT0	=	0x00b9
                           0000B8   392 _PX0	=	0x00b8
                           0000EF   393 _IE_WDOG	=	0x00ef
                           0000EE   394 _IE_GPIO	=	0x00ee
                           0000ED   395 _IE_PWMX	=	0x00ed
                           0000EC   396 _IE_UART1	=	0x00ec
                           0000EB   397 _IE_ADC	=	0x00eb
                           0000EA   398 _IE_USB	=	0x00ea
                           0000E9   399 _IE_TKEY	=	0x00e9
                           0000E8   400 _IE_SPI0	=	0x00e8
                           000097   401 _SCK	=	0x0097
                           000097   402 _TXD1	=	0x0097
                           000097   403 _TIN5	=	0x0097
                           000096   404 _MISO	=	0x0096
                           000096   405 _RXD1	=	0x0096
                           000096   406 _TIN4	=	0x0096
                           000095   407 _MOSI	=	0x0095
                           000095   408 _PWM1	=	0x0095
                           000095   409 _TIN3	=	0x0095
                           000095   410 _UCC2	=	0x0095
                           000095   411 _AIN2	=	0x0095
                           000094   412 _T2_	=	0x0094
                           000094   413 _CAP1_	=	0x0094
                           000094   414 _SCS	=	0x0094
                           000094   415 _TIN2	=	0x0094
                           000094   416 _UCC1	=	0x0094
                           000094   417 _AIN1	=	0x0094
                           000093   418 _TXD_	=	0x0093
                           000092   419 _RXD_	=	0x0092
                           000091   420 _T2EX	=	0x0091
                           000091   421 _CAP2	=	0x0091
                           000091   422 _TIN1	=	0x0091
                           000091   423 _VBUS2	=	0x0091
                           000091   424 _AIN0	=	0x0091
                           000090   425 _T2	=	0x0090
                           000090   426 _CAP1	=	0x0090
                           000090   427 _TIN0	=	0x0090
                           0000B7   428 _UDM	=	0x00b7
                           0000B6   429 _UDP	=	0x00b6
                           0000B5   430 _T1	=	0x00b5
                           0000B4   431 _PWM2	=	0x00b4
                           0000B4   432 _RXD1_	=	0x00b4
                           0000B4   433 _T0	=	0x00b4
                           0000B3   434 _INT1	=	0x00b3
                           0000B2   435 _TXD1_	=	0x00b2
                           0000B2   436 _INT0	=	0x00b2
                           0000B2   437 _VBUS1	=	0x00b2
                           0000B2   438 _AIN3	=	0x00b2
                           0000B1   439 _PWM2_	=	0x00b1
                           0000B1   440 _TXD	=	0x00b1
                           0000B0   441 _PWM1_	=	0x00b0
                           0000B0   442 _RXD	=	0x00b0
                           00008F   443 _TF1	=	0x008f
                           00008E   444 _TR1	=	0x008e
                           00008D   445 _TF0	=	0x008d
                           00008C   446 _TR0	=	0x008c
                           00008B   447 _IE1	=	0x008b
                           00008A   448 _IT1	=	0x008a
                           000089   449 _IE0	=	0x0089
                           000088   450 _IT0	=	0x0088
                           00009F   451 _SM0	=	0x009f
                           00009E   452 _SM1	=	0x009e
                           00009D   453 _SM2	=	0x009d
                           00009C   454 _REN	=	0x009c
                           00009B   455 _TB8	=	0x009b
                           00009A   456 _RB8	=	0x009a
                           000099   457 _TI	=	0x0099
                           000098   458 _RI	=	0x0098
                           0000CF   459 _TF2	=	0x00cf
                           0000CF   460 _CAP1F	=	0x00cf
                           0000CE   461 _EXF2	=	0x00ce
                           0000CD   462 _RCLK	=	0x00cd
                           0000CC   463 _TCLK	=	0x00cc
                           0000CB   464 _EXEN2	=	0x00cb
                           0000CA   465 _TR2	=	0x00ca
                           0000C9   466 _C_T2	=	0x00c9
                           0000C8   467 _CP_RL2	=	0x00c8
                           0000FF   468 _S0_FST_ACT	=	0x00ff
                           0000FE   469 _S0_IF_OV	=	0x00fe
                           0000FD   470 _S0_IF_FIRST	=	0x00fd
                           0000FC   471 _S0_IF_BYTE	=	0x00fc
                           0000FB   472 _S0_FREE	=	0x00fb
                           0000FA   473 _S0_T_FIFO	=	0x00fa
                           0000F8   474 _S0_R_FIFO	=	0x00f8
                           0000C7   475 _U1SM0	=	0x00c7
                           0000C5   476 _U1SMOD	=	0x00c5
                           0000C4   477 _U1REN	=	0x00c4
                           0000C3   478 _U1TB8	=	0x00c3
                           0000C2   479 _U1RB8	=	0x00c2
                           0000C1   480 _U1TI	=	0x00c1
                           0000C0   481 _U1RI	=	0x00c0
                           000087   482 _CMPO	=	0x0087
                           000086   483 _CMP_IF	=	0x0086
                           000085   484 _ADC_IF	=	0x0085
                           000084   485 _ADC_START	=	0x0084
                           000083   486 _CMP_CHAN	=	0x0083
                           000081   487 _ADC_CHAN1	=	0x0081
                           000080   488 _ADC_CHAN0	=	0x0080
                           0000DF   489 _U_IS_NAK	=	0x00df
                           0000DE   490 _U_TOG_OK	=	0x00de
                           0000DD   491 _U_SIE_FREE	=	0x00dd
                           0000DC   492 _UIF_FIFO_OV	=	0x00dc
                           0000DB   493 _UIF_HST_SOF	=	0x00db
                           0000DA   494 _UIF_SUSPEND	=	0x00da
                           0000D9   495 _UIF_TRANSFER	=	0x00d9
                           0000D8   496 _UIF_DETECT	=	0x00d8
                           0000D8   497 _UIF_BUS_RST	=	0x00d8
                                    498 ;--------------------------------------------------------
                                    499 ; overlayable register banks
                                    500 ;--------------------------------------------------------
                                    501 	.area REG_BANK_0	(REL,OVR,DATA)
      000000                        502 	.ds 8
                                    503 ;--------------------------------------------------------
                                    504 ; internal ram data
                                    505 ;--------------------------------------------------------
                                    506 	.area DSEG    (DATA)
      000022                        507 _read_line_PARM_2:
      000022                        508 	.ds 1
      000023                        509 _read_line_buf_10000_60:
      000023                        510 	.ds 3
      000026                        511 _main_buf_10000_72:
      000026                        512 	.ds 32
                                    513 ;--------------------------------------------------------
                                    514 ; overlayable items in internal ram
                                    515 ;--------------------------------------------------------
                                    516 	.area	OSEG    (OVR,DATA)
                                    517 	.area	OSEG    (OVR,DATA)
      000012                        518 _str_equal_PARM_2:
      000012                        519 	.ds 3
      000015                        520 _str_equal_sloc0_1_0:
      000015                        521 	.ds 3
                                    522 ;--------------------------------------------------------
                                    523 ; Stack segment in internal ram
                                    524 ;--------------------------------------------------------
                                    525 	.area SSEG
      000046                        526 __start__stack:
      000046                        527 	.ds	1
                                    528 
                                    529 ;--------------------------------------------------------
                                    530 ; indirectly addressable internal ram data
                                    531 ;--------------------------------------------------------
                                    532 	.area ISEG    (DATA)
                                    533 ;--------------------------------------------------------
                                    534 ; absolute internal ram data
                                    535 ;--------------------------------------------------------
                                    536 	.area IABS    (ABS,DATA)
                                    537 	.area IABS    (ABS,DATA)
                                    538 ;--------------------------------------------------------
                                    539 ; bit data
                                    540 ;--------------------------------------------------------
                                    541 	.area BSEG    (BIT)
                                    542 ;--------------------------------------------------------
                                    543 ; paged external ram data
                                    544 ;--------------------------------------------------------
                                    545 	.area PSEG    (PAG,XDATA)
                                    546 ;--------------------------------------------------------
                                    547 ; uninitialized external ram data
                                    548 ;--------------------------------------------------------
                                    549 	.area XSEG    (XDATA)
                                    550 ;--------------------------------------------------------
                                    551 ; absolute external ram data
                                    552 ;--------------------------------------------------------
                                    553 	.area XABS    (ABS,XDATA)
                                    554 ;--------------------------------------------------------
                                    555 ; initialized external ram data
                                    556 ;--------------------------------------------------------
                                    557 	.area XISEG   (XDATA)
                                    558 	.area HOME    (CODE)
                                    559 	.area GSINIT0 (CODE)
                                    560 	.area GSINIT1 (CODE)
                                    561 	.area GSINIT2 (CODE)
                                    562 	.area GSINIT3 (CODE)
                                    563 	.area GSINIT4 (CODE)
                                    564 	.area GSINIT5 (CODE)
                                    565 	.area GSINIT  (CODE)
                                    566 	.area GSFINAL (CODE)
                                    567 	.area CSEG    (CODE)
                                    568 ;--------------------------------------------------------
                                    569 ; interrupt vector
                                    570 ;--------------------------------------------------------
                                    571 	.area HOME    (CODE)
      000000                        572 __interrupt_vect:
      000000 02 00 B6         [24]  573 	ljmp	__sdcc_gsinit_startup
      000003 32               [24]  574 	reti
      000004                        575 	.ds	7
      00000B 32               [24]  576 	reti
      00000C                        577 	.ds	7
      000013 32               [24]  578 	reti
      000014                        579 	.ds	7
      00001B 32               [24]  580 	reti
      00001C                        581 	.ds	7
      000023 32               [24]  582 	reti
      000024                        583 	.ds	7
      00002B 32               [24]  584 	reti
      00002C                        585 	.ds	7
      000033 32               [24]  586 	reti
      000034                        587 	.ds	7
      00003B 32               [24]  588 	reti
      00003C                        589 	.ds	7
      000043 02 04 12         [24]  590 	ljmp	_USB_interrupt
                                    591 ; restartable atomic support routines
      000046                        592 	.ds	2
      000048                        593 sdcc_atomic_exchange_rollback_start::
      000048 00               [12]  594 	nop
      000049 00               [12]  595 	nop
      00004A                        596 sdcc_atomic_exchange_pdata_impl:
      00004A E2               [24]  597 	movx	a, @r0
      00004B FB               [12]  598 	mov	r3, a
      00004C EA               [12]  599 	mov	a, r2
      00004D F2               [24]  600 	movx	@r0, a
      00004E 80 2C            [24]  601 	sjmp	sdcc_atomic_exchange_exit
      000050 00               [12]  602 	nop
      000051 00               [12]  603 	nop
      000052                        604 sdcc_atomic_exchange_xdata_impl:
      000052 E0               [24]  605 	movx	a, @dptr
      000053 FB               [12]  606 	mov	r3, a
      000054 EA               [12]  607 	mov	a, r2
      000055 F0               [24]  608 	movx	@dptr, a
      000056 80 24            [24]  609 	sjmp	sdcc_atomic_exchange_exit
      000058                        610 sdcc_atomic_compare_exchange_idata_impl:
      000058 E6               [12]  611 	mov	a, @r0
      000059 B5 02 02         [24]  612 	cjne	a, ar2, .+#5
      00005C EB               [12]  613 	mov	a, r3
      00005D F6               [12]  614 	mov	@r0, a
      00005E 22               [24]  615 	ret
      00005F 00               [12]  616 	nop
      000060                        617 sdcc_atomic_compare_exchange_pdata_impl:
      000060 E2               [24]  618 	movx	a, @r0
      000061 B5 02 02         [24]  619 	cjne	a, ar2, .+#5
      000064 EB               [12]  620 	mov	a, r3
      000065 F2               [24]  621 	movx	@r0, a
      000066 22               [24]  622 	ret
      000067 00               [12]  623 	nop
      000068                        624 sdcc_atomic_compare_exchange_xdata_impl:
      000068 E0               [24]  625 	movx	a, @dptr
      000069 B5 02 02         [24]  626 	cjne	a, ar2, .+#5
      00006C EB               [12]  627 	mov	a, r3
      00006D F0               [24]  628 	movx	@dptr, a
      00006E 22               [24]  629 	ret
      00006F                        630 sdcc_atomic_exchange_rollback_end::
                                    631 
      00006F                        632 sdcc_atomic_exchange_gptr_impl::
      00006F 30 F6 E0         [24]  633 	jnb	b.6, sdcc_atomic_exchange_xdata_impl
      000072 A8 82            [24]  634 	mov	r0, dpl
      000074 20 F5 D3         [24]  635 	jb	b.5, sdcc_atomic_exchange_pdata_impl
      000077                        636 sdcc_atomic_exchange_idata_impl:
      000077 EA               [12]  637 	mov	a, r2
      000078 C6               [12]  638 	xch	a, @r0
      000079 F5 82            [12]  639 	mov	dpl, a
      00007B 22               [24]  640 	ret
      00007C                        641 sdcc_atomic_exchange_exit:
      00007C 8B 82            [24]  642 	mov	dpl, r3
      00007E 22               [24]  643 	ret
      00007F                        644 sdcc_atomic_compare_exchange_gptr_impl::
      00007F 30 F6 E6         [24]  645 	jnb	b.6, sdcc_atomic_compare_exchange_xdata_impl
      000082 A8 82            [24]  646 	mov	r0, dpl
      000084 20 F5 D9         [24]  647 	jb	b.5, sdcc_atomic_compare_exchange_pdata_impl
      000087 80 CF            [24]  648 	sjmp	sdcc_atomic_compare_exchange_idata_impl
                                    649 ;--------------------------------------------------------
                                    650 ; global & static initialisations
                                    651 ;--------------------------------------------------------
                                    652 	.area HOME    (CODE)
                                    653 	.area GSINIT  (CODE)
                                    654 	.area GSFINAL (CODE)
                                    655 	.area GSINIT  (CODE)
                                    656 	.globl __sdcc_gsinit_startup
                                    657 	.globl __sdcc_program_startup
                                    658 	.globl __start__stack
                                    659 	.globl __mcs51_genXINIT
                                    660 	.globl __mcs51_genXRAMCLEAR
                                    661 	.globl __mcs51_genRAMCLEAR
                                    662 	.area GSFINAL (CODE)
      000112 02 00 89         [24]  663 	ljmp	__sdcc_program_startup
                                    664 ;--------------------------------------------------------
                                    665 ; Home
                                    666 ;--------------------------------------------------------
                                    667 	.area HOME    (CODE)
                                    668 	.area HOME    (CODE)
      000089                        669 __sdcc_program_startup:
      000089 02 08 FC         [24]  670 	ljmp	_main
                                    671 ;	return from main will return to caller
                                    672 ;--------------------------------------------------------
                                    673 ; code
                                    674 ;--------------------------------------------------------
                                    675 	.area CSEG    (CODE)
                                    676 ;------------------------------------------------------------
                                    677 ;Allocation info for local variables in function 'ch552g_clock_24m'
                                    678 ;------------------------------------------------------------
                                    679 ;	./ch552g/ch552g.h:6: static inline void ch552g_clock_24m(void) {
                                    680 ;	-----------------------------------------
                                    681 ;	 function ch552g_clock_24m
                                    682 ;	-----------------------------------------
      000775                        683 _ch552g_clock_24m:
                           000007   684 	ar7 = 0x07
                           000006   685 	ar6 = 0x06
                           000005   686 	ar5 = 0x05
                           000004   687 	ar4 = 0x04
                           000003   688 	ar3 = 0x03
                           000002   689 	ar2 = 0x02
                           000001   690 	ar1 = 0x01
                           000000   691 	ar0 = 0x00
                                    692 ;	./ch552g/ch552g.h:7: SAFE_MOD = 0x55;
      000775 75 A1 55         [24]  693 	mov	_SAFE_MOD,#0x55
                                    694 ;	./ch552g/ch552g.h:8: SAFE_MOD = 0xAA;
      000778 75 A1 AA         [24]  695 	mov	_SAFE_MOD,#0xaa
                                    696 ;	./ch552g/ch552g.h:9: CLOCK_CFG = (CLOCK_CFG & ~MASK_SYS_CK_SEL) | 0x06;
      00077B 74 F8            [12]  697 	mov	a,#0xf8
      00077D 55 B9            [12]  698 	anl	a,_CLOCK_CFG
      00077F 44 06            [12]  699 	orl	a,#0x06
      000781 F5 B9            [12]  700 	mov	_CLOCK_CFG,a
                                    701 ;	./ch552g/ch552g.h:10: SAFE_MOD = 0x00;
      000783 75 A1 00         [24]  702 	mov	_SAFE_MOD,#0x00
                                    703 ;	./ch552g/ch552g.h:11: }
      000786 22               [24]  704 	ret
                                    705 ;------------------------------------------------------------
                                    706 ;Allocation info for local variables in function 'delay_ms'
                                    707 ;------------------------------------------------------------
                                    708 ;ms            Allocated to registers 
                                    709 ;i             Allocated to registers r5 
                                    710 ;------------------------------------------------------------
                                    711 ;	../main.c:7: static void delay_ms(unsigned int ms) {
                                    712 ;	-----------------------------------------
                                    713 ;	 function delay_ms
                                    714 ;	-----------------------------------------
      000787                        715 _delay_ms:
      000787 AE 82            [24]  716 	mov	r6, dpl
      000789 AF 83            [24]  717 	mov	r7, dph
                                    718 ;	../main.c:8: while (ms--) {
      00078B                        719 00102$:
      00078B 8E 04            [24]  720 	mov	ar4,r6
      00078D 8F 05            [24]  721 	mov	ar5,r7
      00078F 1E               [12]  722 	dec	r6
      000790 BE FF 01         [24]  723 	cjne	r6,#0xff,00135$
      000793 1F               [12]  724 	dec	r7
      000794                        725 00135$:
      000794 EC               [12]  726 	mov	a,r4
      000795 4D               [12]  727 	orl	a,r5
      000796 60 0A            [24]  728 	jz	00108$
                                    729 ;	../main.c:9: for (unsigned int i = 0; i < 200; i++) {}
      000798 7D 00            [12]  730 	mov	r5,#0x00
      00079A                        731 00106$:
      00079A BD C8 00         [24]  732 	cjne	r5,#0xc8,00137$
      00079D                        733 00137$:
      00079D 50 EC            [24]  734 	jnc	00102$
      00079F 0D               [12]  735 	inc	r5
      0007A0 80 F8            [24]  736 	sjmp	00106$
      0007A2                        737 00108$:
                                    738 ;	../main.c:11: }
      0007A2 22               [24]  739 	ret
                                    740 ;------------------------------------------------------------
                                    741 ;Allocation info for local variables in function 'print'
                                    742 ;------------------------------------------------------------
                                    743 ;s             Allocated to registers 
                                    744 ;------------------------------------------------------------
                                    745 ;	../main.c:14: static void print(const char* s) {
                                    746 ;	-----------------------------------------
                                    747 ;	 function print
                                    748 ;	-----------------------------------------
      0007A3                        749 _print:
      0007A3 AD 82            [24]  750 	mov	r5, dpl
      0007A5 AE 83            [24]  751 	mov	r6, dph
      0007A7 AF F0            [24]  752 	mov	r7, b
                                    753 ;	../main.c:15: while (*s) {
      0007A9                        754 00101$:
      0007A9 8D 82            [24]  755 	mov	dpl,r5
      0007AB 8E 83            [24]  756 	mov	dph,r6
      0007AD 8F F0            [24]  757 	mov	b,r7
      0007AF 12 09 BD         [24]  758 	lcall	__gptrget
      0007B2 FC               [12]  759 	mov	r4,a
      0007B3 60 18            [24]  760 	jz	00103$
                                    761 ;	../main.c:16: serial_write(*s++);
      0007B5 8C 82            [24]  762 	mov	dpl,r4
      0007B7 0D               [12]  763 	inc	r5
      0007B8 BD 00 01         [24]  764 	cjne	r5,#0x00,00120$
      0007BB 0E               [12]  765 	inc	r6
      0007BC                        766 00120$:
      0007BC C0 07            [24]  767 	push	ar7
      0007BE C0 06            [24]  768 	push	ar6
      0007C0 C0 05            [24]  769 	push	ar5
      0007C2 12 01 2D         [24]  770 	lcall	_serial_write
      0007C5 D0 05            [24]  771 	pop	ar5
      0007C7 D0 06            [24]  772 	pop	ar6
      0007C9 D0 07            [24]  773 	pop	ar7
      0007CB 80 DC            [24]  774 	sjmp	00101$
      0007CD                        775 00103$:
                                    776 ;	../main.c:18: CDC_flush();
                                    777 ;	../main.c:19: }
      0007CD 02 04 A8         [24]  778 	ljmp	_CDC_flush
                                    779 ;------------------------------------------------------------
                                    780 ;Allocation info for local variables in function 'read_line'
                                    781 ;------------------------------------------------------------
                                    782 ;maxLen        Allocated with name '_read_line_PARM_2'
                                    783 ;buf           Allocated with name '_read_line_buf_10000_60'
                                    784 ;len           Allocated to registers r4 
                                    785 ;c             Allocated to registers r3 
                                    786 ;------------------------------------------------------------
                                    787 ;	../main.c:22: static unsigned char read_line(char* buf, unsigned char maxLen) {
                                    788 ;	-----------------------------------------
                                    789 ;	 function read_line
                                    790 ;	-----------------------------------------
      0007D0                        791 _read_line:
      0007D0 85 82 23         [24]  792 	mov	_read_line_buf_10000_60,dpl
      0007D3 85 83 24         [24]  793 	mov	(_read_line_buf_10000_60 + 1),dph
      0007D6 85 F0 25         [24]  794 	mov	(_read_line_buf_10000_60 + 2),b
                                    795 ;	../main.c:23: unsigned char len = 0;
      0007D9 7C 00            [12]  796 	mov	r4,#0x00
                                    797 ;	../main.c:24: while (1) {
      0007DB                        798 00114$:
                                    799 ;	../main.c:25: if (serial_available()) {
      0007DB C0 04            [24]  800 	push	ar4
      0007DD 12 01 30         [24]  801 	lcall	_serial_available
      0007E0 E5 82            [12]  802 	mov	a, dpl
      0007E2 D0 04            [24]  803 	pop	ar4
      0007E4 60 F5            [24]  804 	jz	00114$
                                    805 ;	../main.c:26: char c = serial_read();
      0007E6 C0 04            [24]  806 	push	ar4
      0007E8 12 01 37         [24]  807 	lcall	_serial_read
      0007EB AB 82            [24]  808 	mov	r3, dpl
      0007ED D0 04            [24]  809 	pop	ar4
                                    810 ;	../main.c:27: if (c == '\r' || c == '\n') {
      0007EF BB 0D 02         [24]  811 	cjne	r3,#0x0d,00155$
      0007F2 80 03            [24]  812 	sjmp	00101$
      0007F4                        813 00155$:
      0007F4 BB 0A 2A         [24]  814 	cjne	r3,#0x0a,00102$
      0007F7                        815 00101$:
                                    816 ;	../main.c:28: serial_write('\r');
      0007F7 75 82 0D         [24]  817 	mov	dpl, #0x0d
      0007FA C0 04            [24]  818 	push	ar4
      0007FC 12 01 2D         [24]  819 	lcall	_serial_write
                                    820 ;	../main.c:29: serial_write('\n');
      0007FF 75 82 0A         [24]  821 	mov	dpl, #0x0a
      000802 12 01 2D         [24]  822 	lcall	_serial_write
                                    823 ;	../main.c:30: CDC_flush();
      000805 12 04 A8         [24]  824 	lcall	_CDC_flush
      000808 D0 04            [24]  825 	pop	ar4
                                    826 ;	../main.c:31: buf[len] = '\0';
      00080A EC               [12]  827 	mov	a,r4
      00080B 25 23            [12]  828 	add	a, _read_line_buf_10000_60
      00080D F8               [12]  829 	mov	r0,a
      00080E E4               [12]  830 	clr	a
      00080F 35 24            [12]  831 	addc	a, (_read_line_buf_10000_60 + 1)
      000811 F9               [12]  832 	mov	r1,a
      000812 AA 25            [24]  833 	mov	r2,(_read_line_buf_10000_60 + 2)
      000814 88 82            [24]  834 	mov	dpl,r0
      000816 89 83            [24]  835 	mov	dph,r1
      000818 8A F0            [24]  836 	mov	b,r2
      00081A E4               [12]  837 	clr	a
      00081B 12 09 A2         [24]  838 	lcall	__gptrput
                                    839 ;	../main.c:32: return len;
      00081E 8C 82            [24]  840 	mov	dpl, r4
      000820 22               [24]  841 	ret
      000821                        842 00102$:
                                    843 ;	../main.c:34: if (c == 0x7F || c == 0x08) {   // 退格
      000821 BB 7F 02         [24]  844 	cjne	r3,#0x7f,00158$
      000824 80 03            [24]  845 	sjmp	00106$
      000826                        846 00158$:
      000826 BB 08 13         [24]  847 	cjne	r3,#0x08,00107$
      000829                        848 00106$:
                                    849 ;	../main.c:35: if (len > 0) {
      000829 EC               [12]  850 	mov	a,r4
      00082A 60 AF            [24]  851 	jz	00114$
                                    852 ;	../main.c:36: len--;
      00082C 1C               [12]  853 	dec	r4
                                    854 ;	../main.c:37: print("\b \b");
      00082D 90 0B 26         [24]  855 	mov	dptr,#___str_0
      000830 75 F0 80         [24]  856 	mov	b, #0x80
      000833 C0 04            [24]  857 	push	ar4
      000835 12 07 A3         [24]  858 	lcall	_print
      000838 D0 04            [24]  859 	pop	ar4
                                    860 ;	../main.c:39: continue;
      00083A 80 9F            [24]  861 	sjmp	00114$
      00083C                        862 00107$:
                                    863 ;	../main.c:41: if (len < maxLen - 1) {
      00083C A9 22            [24]  864 	mov	r1,_read_line_PARM_2
      00083E 7A 00            [12]  865 	mov	r2,#0x00
      000840 19               [12]  866 	dec	r1
      000841 B9 FF 01         [24]  867 	cjne	r1,#0xff,00162$
      000844 1A               [12]  868 	dec	r2
      000845                        869 00162$:
      000845 8C 00            [24]  870 	mov	ar0,r4
      000847 7F 00            [12]  871 	mov	r7,#0x00
      000849 C3               [12]  872 	clr	c
      00084A E8               [12]  873 	mov	a,r0
      00084B 99               [12]  874 	subb	a,r1
      00084C EF               [12]  875 	mov	a,r7
      00084D 64 80            [12]  876 	xrl	a,#0x80
      00084F 8A F0            [24]  877 	mov	b,r2
      000851 63 F0 80         [24]  878 	xrl	b,#0x80
      000854 95 F0            [12]  879 	subb	a,b
      000856 50 83            [24]  880 	jnc	00114$
                                    881 ;	../main.c:42: buf[len++] = c;
      000858 8C 07            [24]  882 	mov	ar7,r4
      00085A 0C               [12]  883 	inc	r4
      00085B EF               [12]  884 	mov	a,r7
      00085C 25 23            [12]  885 	add	a, _read_line_buf_10000_60
      00085E FF               [12]  886 	mov	r7,a
      00085F E4               [12]  887 	clr	a
      000860 35 24            [12]  888 	addc	a, (_read_line_buf_10000_60 + 1)
      000862 FE               [12]  889 	mov	r6,a
      000863 AD 25            [24]  890 	mov	r5,(_read_line_buf_10000_60 + 2)
      000865 8F 82            [24]  891 	mov	dpl,r7
      000867 8E 83            [24]  892 	mov	dph,r6
      000869 8D F0            [24]  893 	mov	b,r5
      00086B EB               [12]  894 	mov	a,r3
      00086C 12 09 A2         [24]  895 	lcall	__gptrput
                                    896 ;	../main.c:43: serial_write(c);
      00086F 8B 82            [24]  897 	mov	dpl, r3
      000871 C0 04            [24]  898 	push	ar4
      000873 12 01 2D         [24]  899 	lcall	_serial_write
                                    900 ;	../main.c:44: CDC_flush();
      000876 12 04 A8         [24]  901 	lcall	_CDC_flush
      000879 D0 04            [24]  902 	pop	ar4
                                    903 ;	../main.c:48: }
      00087B 02 07 DB         [24]  904 	ljmp	00114$
                                    905 ;------------------------------------------------------------
                                    906 ;Allocation info for local variables in function 'str_equal'
                                    907 ;------------------------------------------------------------
                                    908 ;b             Allocated with name '_str_equal_PARM_2'
                                    909 ;a             Allocated to registers r5 r6 r7 
                                    910 ;sloc0         Allocated with name '_str_equal_sloc0_1_0'
                                    911 ;------------------------------------------------------------
                                    912 ;	../main.c:51: static unsigned char str_equal(const char* a, const char* b) {
                                    913 ;	-----------------------------------------
                                    914 ;	 function str_equal
                                    915 ;	-----------------------------------------
      00087E                        916 _str_equal:
      00087E AD 82            [24]  917 	mov	r5, dpl
      000880 AE 83            [24]  918 	mov	r6, dph
      000882 AF F0            [24]  919 	mov	r7, b
                                    920 ;	../main.c:52: while (*a && *b) {
      000884 8D 02            [24]  921 	mov	ar2,r5
      000886 8E 03            [24]  922 	mov	ar3,r6
      000888 8F 04            [24]  923 	mov	ar4,r7
      00088A 85 12 15         [24]  924 	mov	_str_equal_sloc0_1_0,_str_equal_PARM_2
      00088D 85 13 16         [24]  925 	mov	(_str_equal_sloc0_1_0 + 1),(_str_equal_PARM_2 + 1)
      000890 85 14 17         [24]  926 	mov	(_str_equal_sloc0_1_0 + 2),(_str_equal_PARM_2 + 2)
      000893                        927 00104$:
      000893 8A 82            [24]  928 	mov	dpl,r2
      000895 8B 83            [24]  929 	mov	dph,r3
      000897 8C F0            [24]  930 	mov	b,r4
      000899 12 09 BD         [24]  931 	lcall	__gptrget
      00089C F9               [12]  932 	mov	r1,a
      00089D 60 37            [24]  933 	jz	00106$
      00089F 85 15 82         [24]  934 	mov	dpl,_str_equal_sloc0_1_0
      0008A2 85 16 83         [24]  935 	mov	dph,(_str_equal_sloc0_1_0 + 1)
      0008A5 85 17 F0         [24]  936 	mov	b,(_str_equal_sloc0_1_0 + 2)
      0008A8 12 09 BD         [24]  937 	lcall	__gptrget
      0008AB F8               [12]  938 	mov	r0,a
      0008AC 60 28            [24]  939 	jz	00106$
                                    940 ;	../main.c:53: if (*a != *b) return 0;
      0008AE E9               [12]  941 	mov	a,r1
      0008AF B5 00 02         [24]  942 	cjne	a,ar0,00135$
      0008B2 80 04            [24]  943 	sjmp	00102$
      0008B4                        944 00135$:
      0008B4 75 82 00         [24]  945 	mov	dpl, #0x00
      0008B7 22               [24]  946 	ret
      0008B8                        947 00102$:
                                    948 ;	../main.c:54: a++; b++;
      0008B8 0A               [12]  949 	inc	r2
      0008B9 BA 00 01         [24]  950 	cjne	r2,#0x00,00136$
      0008BC 0B               [12]  951 	inc	r3
      0008BD                        952 00136$:
      0008BD 8A 05            [24]  953 	mov	ar5,r2
      0008BF 8B 06            [24]  954 	mov	ar6,r3
      0008C1 8C 07            [24]  955 	mov	ar7,r4
      0008C3 05 15            [12]  956 	inc	_str_equal_sloc0_1_0
      0008C5 E4               [12]  957 	clr	a
      0008C6 B5 15 02         [24]  958 	cjne	a,_str_equal_sloc0_1_0,00137$
      0008C9 05 16            [12]  959 	inc	(_str_equal_sloc0_1_0 + 1)
      0008CB                        960 00137$:
      0008CB 85 15 12         [24]  961 	mov	_str_equal_PARM_2,_str_equal_sloc0_1_0
      0008CE 85 16 13         [24]  962 	mov	(_str_equal_PARM_2 + 1),(_str_equal_sloc0_1_0 + 1)
      0008D1 85 17 14         [24]  963 	mov	(_str_equal_PARM_2 + 2),(_str_equal_sloc0_1_0 + 2)
      0008D4 80 BD            [24]  964 	sjmp	00104$
      0008D6                        965 00106$:
                                    966 ;	../main.c:56: return *a == *b;
      0008D6 8D 82            [24]  967 	mov	dpl,r5
      0008D8 8E 83            [24]  968 	mov	dph,r6
      0008DA 8F F0            [24]  969 	mov	b,r7
      0008DC 12 09 BD         [24]  970 	lcall	__gptrget
      0008DF FD               [12]  971 	mov	r5,a
      0008E0 AC 12            [24]  972 	mov	r4,_str_equal_PARM_2
      0008E2 AE 13            [24]  973 	mov	r6,(_str_equal_PARM_2 + 1)
      0008E4 AF 14            [24]  974 	mov	r7,(_str_equal_PARM_2 + 2)
      0008E6 8C 82            [24]  975 	mov	dpl,r4
      0008E8 8E 83            [24]  976 	mov	dph,r6
      0008EA 8F F0            [24]  977 	mov	b,r7
      0008EC 12 09 BD         [24]  978 	lcall	__gptrget
      0008EF FC               [12]  979 	mov	r4,a
      0008F0 ED               [12]  980 	mov	a,r5
      0008F1 B5 04 04         [24]  981 	cjne	a,ar4,00138$
      0008F4 74 01            [12]  982 	mov	a,#0x01
      0008F6 80 01            [24]  983 	sjmp	00139$
      0008F8                        984 00138$:
      0008F8 E4               [12]  985 	clr	a
      0008F9                        986 00139$:
      0008F9 F5 82            [12]  987 	mov	dpl,a
                                    988 ;	../main.c:57: }
      0008FB 22               [24]  989 	ret
                                    990 ;------------------------------------------------------------
                                    991 ;Allocation info for local variables in function 'main'
                                    992 ;------------------------------------------------------------
                                    993 ;buf           Allocated with name '_main_buf_10000_72'
                                    994 ;c             Allocated to registers r7 
                                    995 ;len           Allocated to registers 
                                    996 ;------------------------------------------------------------
                                    997 ;	../main.c:59: void main(void) {
                                    998 ;	-----------------------------------------
                                    999 ;	 function main
                                   1000 ;	-----------------------------------------
      0008FC                       1001 _main:
                                   1002 ;	../main.c:63: ch552g_init();
      0008FC 12 06 2F         [24] 1003 	lcall	_ch552g_init
                                   1004 ;	../main.c:64: serial_init();
      0008FF 12 01 2A         [24] 1005 	lcall	_serial_init
                                   1006 ;	../main.c:66: while (!USB_ENUM_OK) {}
      000902                       1007 00101$:
      000902 E5 0E            [12] 1008 	mov	a,_USB_ENUM_OK
      000904 60 FC            [24] 1009 	jz	00101$
                                   1010 ;	../main.c:67: while (!CDC_getDTR()) {}
      000906                       1011 00104$:
      000906 90 00 08         [24] 1012 	mov	dptr,#_CDC_controlLineState
      000909 E0               [24] 1013 	movx	a,@dptr
      00090A 30 E0 F9         [24] 1014 	jnb	acc.0,00104$
                                   1015 ;	../main.c:70: hpm_pin_P1_1_init();
      00090D 12 06 53         [24] 1016 	lcall	_hpm_pin_P1_1_init
                                   1017 ;	../main.c:71: hpm_pin_P1_1_write(1);
      000910 90 00 01         [24] 1018 	mov	dptr,#0x0001
      000913 12 06 5A         [24] 1019 	lcall	_hpm_pin_P1_1_write
                                   1020 ;	../main.c:72: delay_ms(200);
      000916 90 00 C8         [24] 1021 	mov	dptr,#0x00c8
      000919 12 07 87         [24] 1022 	lcall	_delay_ms
                                   1023 ;	../main.c:73: hpm_pin_P1_1_write(0);
      00091C 90 00 00         [24] 1024 	mov	dptr,#0x0000
      00091F 12 06 5A         [24] 1025 	lcall	_hpm_pin_P1_1_write
                                   1026 ;	../main.c:77: while (1) {
      000922                       1027 00113$:
                                   1028 ;	../main.c:78: if (serial_available()) {
      000922 12 01 30         [24] 1029 	lcall	_serial_available
      000925 E5 82            [12] 1030 	mov	a, dpl
      000927 60 F9            [24] 1031 	jz	00113$
                                   1032 ;	../main.c:79: char c = serial_read();
      000929 12 01 37         [24] 1033 	lcall	_serial_read
      00092C AF 82            [24] 1034 	mov	r7, dpl
                                   1035 ;	../main.c:80: if (c == '\r' || c == '\n') break;
      00092E BF 0D 02         [24] 1036 	cjne	r7,#0x0d,00168$
      000931 80 03            [24] 1037 	sjmp	00114$
      000933                       1038 00168$:
      000933 BF 0A EC         [24] 1039 	cjne	r7,#0x0a,00113$
      000936                       1040 00114$:
                                   1041 ;	../main.c:85: print("\r\n");
      000936 90 0B 2A         [24] 1042 	mov	dptr,#___str_1
      000939 75 F0 80         [24] 1043 	mov	b, #0x80
      00093C 12 07 A3         [24] 1044 	lcall	_print
                                   1045 ;	../main.c:86: print("=========================\r\n");
      00093F 90 0B 2D         [24] 1046 	mov	dptr,#___str_2
      000942 75 F0 80         [24] 1047 	mov	b, #0x80
      000945 12 07 A3         [24] 1048 	lcall	_print
                                   1049 ;	../main.c:87: print("  Welcome to HPM Shell\r\n");
      000948 90 0B 49         [24] 1050 	mov	dptr,#___str_3
      00094B 75 F0 80         [24] 1051 	mov	b, #0x80
      00094E 12 07 A3         [24] 1052 	lcall	_print
                                   1053 ;	../main.c:88: print("=========================\r\n");
      000951 90 0B 2D         [24] 1054 	mov	dptr,#___str_2
      000954 75 F0 80         [24] 1055 	mov	b, #0x80
      000957 12 07 A3         [24] 1056 	lcall	_print
                                   1057 ;	../main.c:91: print("Password: ");
      00095A 90 0B 62         [24] 1058 	mov	dptr,#___str_4
      00095D 75 F0 80         [24] 1059 	mov	b, #0x80
      000960 12 07 A3         [24] 1060 	lcall	_print
                                   1061 ;	../main.c:94: unsigned char len = read_line(buf, sizeof(buf));
      000963 75 22 20         [24] 1062 	mov	_read_line_PARM_2,#0x20
      000966 90 00 26         [24] 1063 	mov	dptr,#_main_buf_10000_72
      000969 75 F0 40         [24] 1064 	mov	b, #0x40
      00096C 12 07 D0         [24] 1065 	lcall	_read_line
                                   1066 ;	../main.c:96: if (str_equal(buf, PASSWORD)) {
      00096F 75 12 6D         [24] 1067 	mov	_str_equal_PARM_2,#___str_5
      000972 75 13 0B         [24] 1068 	mov	(_str_equal_PARM_2 + 1),#(___str_5 >> 8)
      000975 75 14 80         [24] 1069 	mov	(_str_equal_PARM_2 + 2),#0x80
      000978 90 00 26         [24] 1070 	mov	dptr,#_main_buf_10000_72
      00097B 75 F0 40         [24] 1071 	mov	b, #0x40
      00097E 12 08 7E         [24] 1072 	lcall	_str_equal
      000981 E5 82            [12] 1073 	mov	a, dpl
      000983 60 0B            [24] 1074 	jz	00116$
                                   1075 ;	../main.c:97: print("ok\r\n");
      000985 90 0B 74         [24] 1076 	mov	dptr,#___str_6
      000988 75 F0 80         [24] 1077 	mov	b, #0x80
      00098B 12 07 A3         [24] 1078 	lcall	_print
      00098E 80 92            [24] 1079 	sjmp	00113$
      000990                       1080 00116$:
                                   1081 ;	../main.c:99: delay_ms(3000);
      000990 90 0B B8         [24] 1082 	mov	dptr,#0x0bb8
      000993 12 07 87         [24] 1083 	lcall	_delay_ms
                                   1084 ;	../main.c:100: print("error\r\n");
      000996 90 0B 79         [24] 1085 	mov	dptr,#___str_7
      000999 75 F0 80         [24] 1086 	mov	b, #0x80
      00099C 12 07 A3         [24] 1087 	lcall	_print
                                   1088 ;	../main.c:103: }
      00099F 02 09 22         [24] 1089 	ljmp	00113$
                                   1090 	.area CSEG    (CODE)
                                   1091 	.area CONST   (CODE)
                                   1092 	.area CONST   (CODE)
      000B26                       1093 ___str_0:
      000B26 08                    1094 	.db 0x08
      000B27 20                    1095 	.ascii " "
      000B28 08                    1096 	.db 0x08
      000B29 00                    1097 	.db 0x00
                                   1098 	.area CSEG    (CODE)
                                   1099 	.area CONST   (CODE)
      000B2A                       1100 ___str_1:
      000B2A 0D                    1101 	.db 0x0d
      000B2B 0A                    1102 	.db 0x0a
      000B2C 00                    1103 	.db 0x00
                                   1104 	.area CSEG    (CODE)
                                   1105 	.area CONST   (CODE)
      000B2D                       1106 ___str_2:
      000B2D 3D 3D 3D 3D 3D 3D 3D  1107 	.ascii "========================="
             3D 3D 3D 3D 3D 3D 3D
             3D 3D 3D 3D 3D 3D 3D
             3D 3D 3D 3D
      000B46 0D                    1108 	.db 0x0d
      000B47 0A                    1109 	.db 0x0a
      000B48 00                    1110 	.db 0x00
                                   1111 	.area CSEG    (CODE)
                                   1112 	.area CONST   (CODE)
      000B49                       1113 ___str_3:
      000B49 20 20 57 65 6C 63 6F  1114 	.ascii "  Welcome to HPM Shell"
             6D 65 20 74 6F 20 48
             50 4D 20 53 68 65 6C
             6C
      000B5F 0D                    1115 	.db 0x0d
      000B60 0A                    1116 	.db 0x0a
      000B61 00                    1117 	.db 0x00
                                   1118 	.area CSEG    (CODE)
                                   1119 	.area CONST   (CODE)
      000B62                       1120 ___str_4:
      000B62 50 61 73 73 77 6F 72  1121 	.ascii "Password: "
             64 3A 20
      000B6C 00                    1122 	.db 0x00
                                   1123 	.area CSEG    (CODE)
                                   1124 	.area CONST   (CODE)
      000B6D                       1125 ___str_5:
      000B6D 68 70 6D 31 32 33     1126 	.ascii "hpm123"
      000B73 00                    1127 	.db 0x00
                                   1128 	.area CSEG    (CODE)
                                   1129 	.area CONST   (CODE)
      000B74                       1130 ___str_6:
      000B74 6F 6B                 1131 	.ascii "ok"
      000B76 0D                    1132 	.db 0x0d
      000B77 0A                    1133 	.db 0x0a
      000B78 00                    1134 	.db 0x00
                                   1135 	.area CSEG    (CODE)
                                   1136 	.area CONST   (CODE)
      000B79                       1137 ___str_7:
      000B79 65 72 72 6F 72        1138 	.ascii "error"
      000B7E 0D                    1139 	.db 0x0d
      000B7F 0A                    1140 	.db 0x0a
      000B80 00                    1141 	.db 0x00
                                   1142 	.area CSEG    (CODE)
                                   1143 	.area XINIT   (CODE)
                                   1144 	.area CABS    (ABS,CODE)
