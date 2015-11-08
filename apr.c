#include "pdp6.h"

Apr apr;

void
decode_ch(void)
{
	int ir6_8;
	ir6_8 = apr.ir>>9 & 7;
	apr.ch_inc = apr.ch_inc_op = apr.ch_n_inc_op = 0;
	apr.ch_load = apr.ch_dep = 0;
	if((apr.ir & 0770000) == 0130000){
		apr.ch_inc = ((ir6_8 & 5) == 4 || ir6_8 == 3) && !apr.chf5;
		apr.ch_inc_op = apr.ch_inc && !apr.chf7;
		apr.ch_n_inc_op = (ir6_8 & 5) == 5 && !apr.chf5 ||
		                  apr.ch_inc && apr.chf7;
		apr.ch_load = (ir6_8 & 6) == 4 && apr.chf5;
		apr.ch_dep = (ir6_8 & 6) == 6 && apr.chf5;
	}
	if(apr.ch_inc_op || apr.ch_n_inc_op || apr.ch_load)
		apr.fac_inh = 1;
	if(apr.ch_n_inc_op || apr.ch_load)
		apr.fc_e = 1;
	if(apr.ch_dep || apr.ch_inc_op)
		apr.fc_e_pse = 1;
	if(apr.ch_dep)
		apr.sac_inh = 1;
}

void
decode_2xx(void)
{
	apr.ir_fwt = (apr.inst & 0760) == 0200;
	apr.ir_fwt_swap = 0;
	apr.fwt_00 = apr.fwt_01 = apr.fwt_10 = apr.fwt_11 = 0;
	if(apr.ir_fwt){
		apr.ir_fwt_swap = (apr.inst & 0774) == 0204;
		apr.fwt_00 = (apr.inst & 03) == 0;
		apr.fwt_01 = (apr.inst & 03) == 1;
		apr.fwt_10 = (apr.inst & 03) == 2;
		apr.fwt_11 = (apr.inst & 03) == 3;
		if(apr.fwt_00 || apr.fwt_01 || apr.fwt_11)
			apr.fac_inh = 1;
		if(apr.fwt_00)
			apr.fc_e = 1;
		if(apr.fwt_11)
			apr.fc_e_pse = 1;
		if(apr.fwt_10)
			apr.sac_inh = apr.sc_e = 1;
	}
	apr.fc_c_acrt = (apr.inst & 0776) == 0262;
	apr.fc_c_aclt = apr.inst == 0251 || apr.inst == 0267;

	apr.ir_md = (apr.inst & 0760) == 0220;
	if(apr.ir_md){
		if((apr.inst & 03) != 1)
			apr.fc_e = 1;
		if((apr.inst & 03) == 2)
			apr.sac_inh = 1;
		if(apr.inst & 02)
			apr.sc_e = 1;
		if((apr.inst & 0774) == 0224)
			apr.fac2 = 1;
		if(((apr.inst & 0774) == 0224 || (apr.inst & 0770) == 0230) &&
		   !apr.sac_inh)
			apr.sac2 = 1;
	}

	if(apr.inst >= 0244 && apr.inst <= 0246)
		apr.fac2 = apr.sac2 = 1;

	if(apr.inst == 0250)	/* EXCH */
		apr.fc_e_pse = 1;
	if((apr.inst & 0774) == 0254)
		apr.fac_inh = 1;
	if(apr.inst == 0256)	/* XCT */
		apr.fac_inh = apr.fc_e = 1;

	apr.ir_jp = (apr.inst & 0770) == 0260;
	if(apr.ir_jp){
		apr.e_long = 1;
		if(apr.inst == 0260)	/* PUSHJ */
			apr.sc_e = apr.mb_pc_sto = 1;
		if(apr.inst == 0261)	/* PUSH */
			apr.sc_e = apr.fc_e = 1;
		if((apr.inst & 0776) == 0264)	/* JSR, JSP */
			apr.mb_pc_sto = apr.fac_inh = 1;
		if(apr.inst == 0262 || apr.inst == 0264 || apr.inst == 266)
			apr.sc_e = 1;
	}

	apr.ir_as = (apr.inst & 0770) == 0270;
}

void
decodeir(void)
{
	bool iot_a, jrst_a, uuo_a;

	apr.inst = apr.ir>>9 & 0777;
	apr.fac_inh = 0;
	apr.fc_e = 0;
	apr.fc_e_pse = 0;
	apr.e_long = 0;
	apr.mb_pc_sto = 0;
	apr.sc_e = 0;
	apr.sac_inh = 0;
	apr.sac2 = 0;

	decode_ch();

	apr.ir_fp = (apr.inst & 0740) == 0140;
	if(apr.ir_fp){
		apr.fc_e = 1;
		if(apr.inst & 02)	/* M, B */
			apr.sc_e = 1;
		if((apr.inst & 03) == 2)
			apr.sac_inh = 1;
		if((apr.inst & 03) == 1)
			apr.sac2 = 1;
	}

	decode_2xx();

	/* ACCP v MEMAC */
	apr.ir_accp = apr.ir_memac = apr.ir_memac_mem = apr.ir_memac_ac = 0;
	if((apr.inst & 0700) == 0300){
		apr.ir_memac = apr.inst & 060;
		apr.ir_accp = !apr.ir_memac;
		apr.ir_memac_mem = apr.ir_memac && apr.inst & 010;
		apr.ir_memac_ac = apr.ir_memac && !apr.ir_memac_mem;
		if(apr.ir_memac_mem)
			apr.fac_inh = apr.fc_e_pse = 1;
		if(apr.ir_accp && apr.inst & 010)
			apr.fc_e = 1;
		apr.e_long = 1;
		if(apr.ir_accp)
			apr.sac_inh = 1;
	}

	/* BOOLE */
	apr.boole_as_00 = apr.boole_as_01 = 0;
	apr.boole_as_10 = apr.boole_as_11 = 0;
	apr.ir_boole = (apr.inst & 0700) == 0400;
	if(apr.ir_boole)
		apr.ir_boole_op = apr.inst>>2 & 017;

	/* HWT */
	apr.hwt_00 = apr.hwt_01 = apr.hwt_10 = apr.hwt_11 = 0;
	apr.ir_hwt = (apr.inst & 0700) == 0500;
	if(apr.ir_hwt){
		apr.hwt_00 = (apr.inst & 03) == 0;
		apr.hwt_01 = (apr.inst & 03) == 1;
		apr.hwt_10 = (apr.inst & 03) == 2;
		apr.hwt_11 = (apr.inst & 03) == 3;
		if(apr.hwt_00)
			apr.fc_e = 1;
		if(apr.hwt_11)
			apr.fac_inh = 1;
		if(apr.hwt_10 || apr.hwt_11)
			apr.fc_e_pse = 1;
		if(apr.hwt_10)
			apr.sac_inh = 1;
	}

	if(apr.ir_boole || apr.ir_as){
		apr.boole_as_00 = (apr.inst & 03) == 0;
		apr.boole_as_01 = (apr.inst & 03) == 1;
		apr.boole_as_10 = (apr.inst & 03) == 2;
		apr.boole_as_11 = (apr.inst & 03) == 3;
		if(apr.boole_as_00)
			apr.fc_e = 1;
		if(apr.boole_as_10 || apr.boole_as_11)
			apr.fc_e_pse = 1;
		if(apr.boole_as_10)
			apr.sac_inh = 1;
	}

	/* ACBM */
	apr.ir_acbm = (apr.inst & 0700) == 0600;
	if(apr.ir_acbm){
		if(apr.inst & 010)
			apr.fc_e = 1;
		apr.e_long = 1;
		if(!(apr.inst & 60))
			apr.sac_inh = 1;
	}

	if(!(apr.ir & 0740) && (apr.fwt_11 || apr.hwt_11 || apr.ir_memac_mem))
		apr.sac_inh = 1;

	uuo_a = (apr.inst & 0700) == 0;
	iot_a = (apr.inst & 0700) == 0700;
	jrst_a = apr.inst == 0254;
	// 5-13
	apr.ex_ir_uuo =
		uuo_a && apr.ex_uuo_sync ||
		iot_a && !apr.ex_pi_sync && apr.ex_user && !apr.cpa_iot_user ||
		jrst_a && (apr.ir & 0000600) && apr.ex_user;
	apr.ir_jrst = !apr.ex_ir_uuo && jrst_a;
	apr.ir_iot = !apr.ex_ir_uuo && iot_a;
	apr.iot_blk = apr.ir_iot && (apr.ir & 0240) == 0;
	apr.iot_dataio = apr.ir_iot && (apr.ir & 0240) == 0040;

	if(apr.ir_jrst)
		apr.mb_pc_sto = 1;

	if(apr.ex_ir_uuo || apr.ir_iot)
		apr.fac_inh = 1;
	if(apr.iot_blk)
		apr.fc_e_pse = 1;
	if(apr.iot_dataio && apr.ir & 0100)		/* DATAO */
		apr.fc_e = 1;
	/* No need to check PC +1 for E LONG, it's already implied.
	 * We have to wait until ET5 for PC SET */
	if(apr.ir_iot && (apr.inst & 0300) == 0300 ||	/* CONSZ, CONSO */
	   apr.mb_pc_sto)
		apr.e_long = 1;
	if(apr.ir_iot && (apr.ir & 0140) == 040)	/* DATAI, CONI */
		apr.sc_e = 1;
}

void
swap(word *a, word *b)
{
	word tmp;
	tmp = *a;
	*a = *b;
	*b = tmp;
}

bool
accp_et_al_test(void)
{
	bool lt, cond;
	lt = !!(apr.ar & SGN) != apr.ar_cry0_xor_cry1;
	cond = (apr.inst & 2) && apr.ar == 0 ||
               (apr.ir_accp || apr.ir_memac) && (apr.inst & 1) && lt;
	return cond != !!(apr.inst & 040);
}

bool
selected_flag(void)
{
	return apr.ir & 0400 && apr.ar_ov_flag ||
	       apr.ir & 0200 && apr.ar_cry0_flag ||
	       apr.ir & 0100 && apr.ar_cry1_flag ||
	       apr.ir & 0040 && apr.ar_pc_chg_flag;
}

bool
pc_set_enable(void)
{
	// 5-12
	return apr.inst == 0252 && !(apr.ar & SGN) ||	/* AOBJP */
	       apr.inst == 0253 && apr.ar & SGN ||	/* AOBJN */
	       apr.ir_memac_ac && accp_et_al_test() ||
	       apr.inst == 0255 && selected_flag();	/* JFCL */
}

bool
pc_inc_enable(void)
{
	return (apr.ir_accp || apr.ir_acbm || apr.ir_memac_mem) && accp_et_al_test() ||
	       apr.ir_iot && ((apr.inst & 0340) == 0300 && apr.ar == 0 ||
	                      (apr.inst & 0340) == 0340 && apr.ar != 0);
}

void
ar_cry(void)
{
	// 6-10
	apr.ar_cry0_xor_cry1 = apr.ar_cry0 != apr.ar_cry1 &&
	                       !((apr.inst & 0700) == 0300 &&
	                         (apr.inst & 0060) != 0020);
}

void
ar_cry_in(word c)
{
	word a;
	a = (apr.ar & 0377777777777) + c;
	apr.ar += c;
	apr.ar_cry0 = !!(apr.ar & 01000000000000);
	apr.ar_cry1 = !!(a      &  0400000000000);
	apr.ar &= FW;
	ar_cry();
}

void
set_ex_mode_sync(bool value)
{
	apr.ex_mode_sync = value;
	if(apr.ex_mode_sync)
		apr.ex_user = 1;	// 5-13
}

void
set_pi_cyc(bool value)
{
	apr.pi_cyc = value;
	if(!apr.pi_cyc)
		apr.ex_pi_sync = 1;	// 5-13
}

void
recalc_pi_req(void)
{
	int chan;
	u8 req;
	// 8-3
	apr.pi_req = 0;
	if(apr.pi_active){
		req = apr.pir & ~apr.pih;
		for(chan = 0100; chan; chan >>= 1)
			if(req & chan){
				apr.pi_req = chan;
				break;
			}
	}
}

void
set_mc_rq(bool value)
{
	apr.mc_rq = value;		// 7-9
	if(value && (apr.mc_rd || apr.mc_wr)){
		membus0 |= MEMBUS_RQ_CYC;
		wakemem();
	}else
		membus0 &= ~MEMBUS_RQ_CYC;
}

void
set_mc_wr(bool value)
{
	apr.mc_wr = value;		// 7-9
	if(value)
		membus0 |= MEMBUS_WR_RQ;
	else
		membus0 &= ~MEMBUS_WR_RQ;
	set_mc_rq(apr.mc_rq);
}

void
set_mc_rd(bool value)
{
	apr.mc_rd = value;		// 7-9
	if(value)
		membus0 |= MEMBUS_RD_RQ;
	else
		membus0 &= ~MEMBUS_RD_RQ;
	set_mc_rq(apr.mc_rq);
}

void
set_key_rim_sbr(bool value)
{
	// not sure if this is correct
	apr.key_rim_sbr = value | apr.sw_rim_maint;	// 5-2
}

bool
calcaddr(void)
{
	u8 ma18_25;
	bool ma_ok, ma_fmc_select;

	// 5-13
	apr.ex_inh_rel = !apr.ex_user || apr.ex_pi_sync ||
	                 (apr.ma & 0777760) == 0 || apr.ex_ill_op;
	// 7-4
	ma18_25 = apr.ma>>10 & 0377;
	ma_ok = ma18_25 <= apr.pr;
	// 7-2
	ma_fmc_select = !apr.key_rim_sbr && (apr.ma & 0777760) == 0;
	// 7-5
	apr.rla = ma18_25;
	if(!apr.ex_inh_rel)
		apr.rla += apr.rlr;

	membus0 &= ~0007777777761;
	membus0 |= ma_fmc_select ? MEMBUS_MA_FMC_SEL1 : MEMBUS_MA_FMC_SEL0;
	membus0 |= (apr.ma&01777) << 4;
	membus0 |= (apr.rla&017) << 14;
	membus0 |= apr.rla & 0020 ? MEMBUS_MA21_1|MEMBUS_MA21 : MEMBUS_MA21_0;
	membus0 |= apr.rla & 0040 ? MEMBUS_MA20_1 : MEMBUS_MA20_0;
	membus0 |= apr.rla & 0100 ? MEMBUS_MA19_1 : MEMBUS_MA19_0;
	membus0 |= apr.rla & 0200 ? MEMBUS_MA18_1 : MEMBUS_MA18_0;
	membus0 |= apr.ma & 01 ? MEMBUS_MA35_1 : MEMBUS_MA35_0;
	return ma_ok;
}

pulse(kt4);
pulse(it1);
pulse(at1);
pulse(at0);
pulse(iat0);
pulse(mc_wr_rs);
pulse(mc_rd_rq_pulse);
pulse(mc_wr_rq_pulse);
pulse(mc_rdwr_rq_pulse);
pulse(mc_rd_wr_rs_pulse);

// TODO: find A LONG

pulse(pi_reset){
	printf("PI RESET\n");
	apr.pi_active = 0;	// 8-4
	apr.pih = 0;		// 8-4
	apr.pir = 0;		// 8-4
	recalc_pi_req();	// 8-4
	apr.pio = 0;		// 8-3
	return NULL;
}

pulse(ar_flag_clr){
	printf("AR FLAG CLR\n");
	apr.ar_pc_chg_flag = 0;	// 6-10
	apr.ar_ov_flag = 0;	// 6-10
	apr.ar_cry0_flag = 0;	// 6-10
	apr.ar_cry1_flag = 0;	// 6-10
	ar_cry();
	apr.chf7 = 0;		// 6-19
	return NULL;
}

// TODO
pulse(mp_clr){
	apr.chf5 = 0;		// 6-19
	return NULL;
}

pulse(mr_clr){
	printf("MR CLR\n");
	apr.ir = 0;	// 5-7
	apr.mq = 0;	// 6-13
	apr.sc = 0;	// 6-15
	apr.fe = 0;	// 6-15

	apr.mc_rd = 0;		// 7-9
	apr.mc_wr = 0;		// 7-9
	apr.mc_rq = 0;		// 7-9
	apr.mc_stop = 0;	// 7-9
	apr.mc_stop_sync = 0;	// 7-9
	apr.mc_split_cyc_sync = 0;	// 7-9

	set_ex_mode_sync(0);	// 5-13
	apr.ex_uuo_sync = 0;	// 5-13
	apr.ex_pi_sync = 0;	// 5-13

	apr.a_long = 0;		// nowhere to be found :(
	apr.ar_com_cont = 0;	// 6-9
	mp_clr();
	// TODO: DS CLR

	/* sbr flip-flops */
	apr.key_rd_wr = 0;	// 5-2
	apr.if1a = 0;		// 5-3
	apr.af0 = 0;		// 5-3
	apr.af3 = 0;		// 5-3
	apr.af3a = 0;		// 5-3
	apr.f1a = 0;		// 5-4
	apr.f4a = 0;		// 5-4
	apr.f6a = 0;		// 5-4
	apr.et4_ar_pse = 0;	// 5-5
	apr.mc_rst1_ret = NULL;
	apr.art3_ret = NULL;
	return NULL;
}

pulse(mr_start){
	printf("MR START\n");
	// IOB RESET
	apr.cpa_iot_user = 0;	// 8-5
	apr.cpa_illeg_op = 0;	// 8-5
	apr.cpa_non_exist_mem = 0;	// 8-5
	apr.cpa_clock_en = 0;	// 8-5
	apr.cpa_clock_flag = 0;	// 8-5
	apr.cpa_pc_chg_en = 0;	// 8-5
	apr.cpa_pdl_ov = 0;	// 8-5
	apr.cpa_arov_en = 0;	// 8-5
	apr.cpa_pia33 = 0;	// 8-5
	apr.cpa_pia34 = 0;	// 8-5
	apr.cpa_pia35 = 0;	// 8-5
	apr.pi_ov = 0;		// 8-4
	set_pi_cyc(0);		// 8-4
	pi_reset();		// 8-4
	ar_flag_clr();		// 6-10
	apr.ex_user = 0;	// 5-13
	apr.ex_ill_op = 0;	// 5-13
	apr.pr = 0;		// 5-13, 7-4
	apr.rlr = 0;		// 5-13, 7-5
	apr.rla = 0;

//	apr.ex_user = 1;
//	apr.rlr = 2;
//	apr.pr = 1;
	return NULL;
}

pulse(mr_pwr_clr){
	printf("MR PWR CLR\n");
	apr.run = 0;	// 5-1
	/* order matters because of EX PI SYNC */
	mr_start();	// 5-2
	mr_clr();	// 5-2
	return NULL;
}

pulse(st7){
	printf("ST7\n");
	return NULL;
}

/*
 * AR subroutines
 */

pulse(art3){
	printf("ART3\n");
	apr.ar_com_cont = 0;	// 6-9
	return apr.art3_ret;
}

pulse(ar_cry_comp){
	printf("AR CRY COMP\n");
	apr.ar = ~apr.ar & FW;	// 6-8
	return art3;		// 6-9
}

pulse(ar_pm1_t1){
	printf("AR AR+-1 T1\n");
	ar_cry_in(1);
	if(apr.iot_blk || apr.inst == 0252 || apr.inst == 0253 ||
	   apr.ir_jp && !(apr.inst & 0004))
		ar_cry_in(01000000);
	if(apr.ar_com_cont)
		return ar_cry_comp;
	return art3;
}

pulse(ar_pm1_t0){
	printf("AR AR+-1 T0\n");
	apr.ar = ~apr.ar & FW;	// 6-8
	apr.ar_com_cont = 1;	// 6-9
	return ar_pm1_t1;
}

pulse(ar_negate_t0){
	printf("AR NEGATE T0\n");
	apr.ar = ~apr.ar & FW;	// 6-8
	return ar_pm1_t1;
}

pulse(ar_ast2){
	printf("AR2\n");
	ar_cry_in((~apr.ar & apr.mb) << 1);
	if(apr.ar_com_cont)
		return ar_cry_comp;
	return art3;
}

pulse(ar_ast1){
	printf("AR AST1\n");
	apr.ar ^= apr.mb;
	return ar_ast2;
}

pulse(ar_ast0){
	printf("AR AST0\n");
	apr.ar = ~apr.ar & FW;	// 6-8
	apr.ar_com_cont = 1;	// 6-9
	return ar_ast1;
}

/*
 * Priority Interrupt
 */

pulse(pir_stb){
	printf("PIR STB\n");
	apr.pir |= apr.pio & iobus1;
	recalc_pi_req();			// 8-3
	return NULL;
}

pulse(pi_sync){
	printf("PI SYNC\n");
	if(!apr.pi_cyc)
		pir_stb();
	// 5-3
	if(apr.pi_req && !apr.pi_cyc)
		return iat0;
	// TODO: IA INH/AT INH
	if(apr.if1a)
		return it1;
	return at1;
}

/*
 * Store
 */

pulse(st3){
	printf("ST3\n");
	apr.sf3 = 0;
	return NULL;
}

pulse(st2){
	printf("ST2\n");
	apr.sf3 = 1;
	apr.mc_rst1_ret = st3;
	return mc_rd_wr_rs_pulse;
}

pulse(st1){
	printf("ST1\n");
	apr.sf3 = 1;
	apr.mc_rst1_ret = st3;
	return mc_wr_rq_pulse;
}

/*
 * Execute
 */

pulse(et10){
	printf("ET10\n");

	if(apr.pi_hold){
		apr.pi_ov = 0;
		apr.pi_cyc = 0;
	}
	if(apr.ir_jrst || apr.inst == 0260 || apr.inst == 0261)
		apr.ma |= apr.mb & RT;
	// TODO: MB <- AR(J)
	if(apr.ir_jp && !(apr.inst & 04))
		swap(&apr.mb, &apr.ar);
	if(apr.ir_memac || apr.ir_as){
		apr.ar_cry0_flag = apr.ar_cry0;
		apr.ar_cry1_flag = apr.ar_cry1;
	}

	if(apr.hwt_10 || apr.hwt_11 || apr.fwt_10 || apr.fwt_11 ||
	   apr.ir_memac_mem)
		apr.mb = apr.ar;
	if(apr.ir_fwt && !apr.ar_cry0 && apr.ar_cry1 ||
	   (apr.ir_memac || apr.ir_as) && apr.ar_cry0 != apr.ar_cry1){
		apr.ar_ov_flag = 1;			// 6-10
		if(apr.cpa_arov_en)
			;
	}
	if(apr.ir_jp && !(apr.inst & 04) && apr.ar_cry0){
		apr.cpa_pdl_ov = 1;
		;
	}
	if(apr.sc_e)
		return st1;
	if(apr.fc_e_pse)
		return st2;
	return st3;
}

pulse(et9){
	bool pc_inc;

	printf("ET9\n");
	pc_inc = apr.inst == 0264 || apr.inst == 0266 || pc_inc_enable();
	if(pc_inc)
		apr.pc = apr.pc+1 & FW;
	if((apr.pc_set || pc_inc) && !(apr.ir_jrst && apr.ir & 0100)){
		apr.ar_pc_chg_flag = 1;
		if(apr.cpa_pc_chg_en)
			;
	}

	if(apr.ir_acbm || apr.ir_jp && apr.inst != 0264)
		swap(&apr.mb, &apr.ar);
	if(apr.ir_jrst || apr.inst == 0260 || apr.inst == 0261)
		apr.ma = 0;
	return et10;
}

pulse(et8){
	printf("ET8\n");
	if(apr.pc_set)
		apr.pc |= apr.ma;	// 5-12
	return et9;
}

pulse(et7){
	printf("ET7\n");
	if(apr.pc_set)
		apr.pc = 0;		// 5-12
	if(apr.inst == 0264 && (apr.ex_pi_sync || apr.ex_ill_op))
		apr.ex_user = 0;	// 5-13

	if(apr.ir_acbm)
		apr.ar = ~apr.ar;
	return et8;
}

pulse(et6){
	printf("ET6\n");
	if(apr.mb_pc_sto || apr.inst == 0266)
		apr.mb |= apr.pc;
	if(apr.inst == 0261 || apr.inst == 0264 || apr.inst == 0265){
		// 6-4
		if(apr.ar_ov_flag)     apr.mb |= 0400000000000;
		if(apr.ar_cry0_flag)   apr.mb |= 0200000000000;
		if(apr.ar_cry1_flag)   apr.mb |= 0100000000000;
		if(apr.ar_pc_chg_flag) apr.mb |= 0040000000000;
		if(apr.chf7)           apr.mb |= 0020000000000;
		if(apr.ex_user)        apr.mb |= 0010000000000;
	}

	if(apr.ir_acbm && (apr.inst & 060) == 020)	/* Z */
		apr.mb &= apr.ar;
	return et7;
}

pulse(et5){
	printf("ET5\n");
	if(apr.mb_pc_sto)
		apr.mb = 0;

	if(apr.ir_acbm)
		apr.ar = ~apr.ar;

	// 5-12
	apr.pc_set = apr.ir_jp && !(apr.inst == 0261 || apr.inst == 0262) ||	/* JMP */
	             apr.ir_jrst ||
	             pc_set_enable();
	if(apr.e_long | apr.pc_set)
		return et6;
	return et10;
}

pulse(et4){
	bool hwt_lt, hwt_rt;

	printf("ET4\n");
	apr.et4_ar_pse = 0;		// 5-5

	if(apr.ir_boole && (apr.ir_boole_op == 04 ||
	                    apr.ir_boole_op == 010 ||
	                    apr.ir_boole_op == 011 ||
	                    apr.ir_boole_op == 014 ||
	                    apr.ir_boole_op == 015 ||
	                    apr.ir_boole_op == 016 ||
	                    apr.ir_boole_op == 017))
		apr.ar = ~apr.ar & FW;	// 6-8

	hwt_lt = apr.ir_hwt && !(apr.inst & 040);
	hwt_rt = apr.ir_hwt && apr.inst & 040;
	if(hwt_rt && apr.inst & 020 && (!(apr.inst & 010) || apr.mb & RSGN))
		apr.ar = apr.ar & ~LT | ~apr.ar & LT;
	if(hwt_lt && apr.inst & 020 && (!(apr.inst & 010) || apr.mb & SGN))
		apr.ar = apr.ar & ~RT | ~apr.ar & RT;
	if(hwt_lt)
		apr.ar = apr.ar & ~LT | apr.mb & LT;
	if(hwt_rt)
		apr.ar = apr.ar & ~RT | apr.mb & RT;

	if(apr.ir_fwt_swap)
		swap(&apr.mb, &apr.ar);	// 6-3

	if(apr.ir_acbm)
		swap(&apr.mb, &apr.ar);
	return et5;
}

pulse(et3){
	printf("ET3\n");

	// 5-9
	if(apr.ir_fwt && ((apr.inst & 0774) == 0210 ||
	                  (apr.inst & 0774) == 0214 && apr.ar & SGN)){
		apr.et4_ar_pse = 1;		// 5-5
		apr.art3_ret = et4;
		return ar_negate_t0;
	}

	if(apr.ir_as){
		apr.et4_ar_pse = 1;		// 5-5
		apr.art3_ret = et4;
		if(apr.inst & 4)
			return ar_ast0;
		else
			return ar_ast1;
	}

	if(apr.ir_accp){
		apr.et4_ar_pse = 1;		// 5-5
		apr.art3_ret = et4;
		return ar_ast0;
	}
	if(apr.ir_memac){
		apr.et4_ar_pse = 1;		// 5-5
		apr.art3_ret = et4;
		if((apr.inst & 070) == 040)	// +1
			return ar_pm1_t1;
		if((apr.inst & 070) == 060)	// -1
			return ar_pm1_t0;
	}
	if(apr.inst == 0252 || apr.inst == 0253){	/* AOBJ[PN] */
		apr.et4_ar_pse = 1;		// 5-5
		apr.art3_ret = et4;
		return ar_pm1_t1;
	}
	return et4;
};

pulse(et1){
	printf("ET1\n");

	if(apr.ex_ir_uuo)
		apr.ex_ill_op = 1;		// 5-13
	if(apr.ir_jrst && apr.ir & 0000040)
		apr.ex_mode_sync = 1;		// 5-13
	if(apr.ir_jrst && apr.ir & 0000400 ||
	   apr.iot_dataio && !apr.pi_ov && apr.pi_cyc)
		if(apr.pi_active){
			// TODO: check if this correct
			apr.pih &= apr.pi_req-1;	// 8-3
			recalc_pi_req();
		}

	if(apr.ir_boole && (apr.ir_boole_op == 06 ||
	                    apr.ir_boole_op == 011 ||
	                    apr.ir_boole_op == 014))
		apr.ar ^= apr.mb;		// 6-8
	if(apr.ir_boole && (apr.ir_boole_op == 01 ||
	                    apr.ir_boole_op == 02 ||
	                    apr.ir_boole_op == 015 ||
	                    apr.ir_boole_op == 016))
		apr.ar &= apr.mb;		// 6-8
	if(apr.ir_boole && (apr.ir_boole_op == 03 ||
	                    apr.ir_boole_op == 04 ||
	                    apr.ir_boole_op == 07 ||
	                    apr.ir_boole_op == 010 ||
	                    apr.ir_boole_op == 013))
		apr.ar |= apr.mb;		// 6-8
	if(apr.ir_fwt_swap ||
	   apr.ir_hwt && apr.inst & 04)
		apr.mb = apr.mb<<18 & LT | apr.mb>>18 & RT;	// 6-3
	if(apr.ir_hwt && apr.inst & 030)
		apr.ar = 0;			// 6-8

	if(apr.ir_acbm){
		apr.mb &= apr.ar;
		if((apr.inst & 060) == 040)	/* C */
			apr.ar ^= apr.mb;
		if((apr.inst & 060) == 060)	/* O */
			apr.ar |= apr.mb;
	}
	return et3;
}

pulse(et0){
	printf("ET0\n");

	if(!(apr.ch_inc_op && apr.inst != 0133 ||
	     apr.ch_n_inc_op && apr.inst != 0133 ||
	     apr.ex_ir_uuo || apr.iot_blk || apr.inst == 0256 ||
	     apr.key_execute && !apr.run || apr.pi_cyc))
		apr.pc = apr.pc+1 & RT;
	apr.ar_cry0 = 0;	// 6-10
	apr.ar_cry1 = 0;	// 6-10
	ar_cry();
	// TODO: subroutines
	return et1;
}

pulse(et0a){
	printf("ET0A\n");

	// 8-4
	apr.pi_hold = (!apr.ir_iot || !apr.pi_ov && apr.iot_dataio) &&
	              apr.pi_cyc;
	if(apr.pi_hold){
		apr.pih |= apr.pi_req;	// 8-3
		recalc_pi_req();
	}
	// 5-1
	if(apr.key_ex_sync)
		apr.key_ex_st = 1;
	if(apr.key_dep_sync)
		apr.key_dep_st = 1;
	if(apr.key_inst_stop ||
	   apr.ir_jrst && (apr.ir & 0200) && !apr.ex_user)
		apr.run = 0;

	if(apr.ir_boole && (apr.ir_boole_op == 00 ||
	                    apr.ir_boole_op == 03 ||
	                    apr.ir_boole_op == 014 ||
	                    apr.ir_boole_op == 017))
		apr.ar = 0;		// 6-8
	if(apr.ir_boole && (apr.ir_boole_op == 02 ||
	                    apr.ir_boole_op == 04 ||
	                    apr.ir_boole_op == 012 ||
	                    apr.ir_boole_op == 013 ||
	                    apr.ir_boole_op == 015))
		apr.ar = ~apr.ar & FW;	// 6-8
	if(apr.fwt_00 || apr.fwt_11 || apr.hwt_11 || apr.ir_memac_mem)
		apr.ar = apr.mb;	// 6-8
	if(apr.fwt_01 || apr.fwt_10)
		apr.mb = apr.ar;	// 6-3
	if(apr.inst == 0250 ||		/* EXCH */
	   apr.hwt_10)
		swap(&apr.mb, &apr.ar);	// 6-3

	if(apr.ir_acbm && apr.inst & 1)
		apr.mb = apr.mb<<18 & LT | apr.mb>>18 & RT;	// 6-3
	return NULL;
}

/*
 * Fetch
 */

pulse(ft6a){
	printf("FT6A\n");
	apr.f6a = 0;			// 5-4
	et0a();				// 5-5
	return et0;			// 5-5
}

pulse(ft7){
	printf("FT7\n");
	apr.f6a = 1;			// 5-4
	apr.mc_rst1_ret = ft6a;
	if(apr.mc_split_cyc_sync)
		return mc_rd_rq_pulse;
	return mc_rdwr_rq_pulse;
}

pulse(ft6){
	printf("FT6\n");
	apr.f6a = 1;			// 5-4
	apr.mc_rst1_ret = ft6a;
	return mc_rd_rq_pulse;
}

pulse(ft5){
	printf("FT5\n");
	apr.ma = apr.mb & RT;		// 7-3
	if(apr.fc_e)
		return ft6;
	if(apr.fc_e_pse)
		return ft7;
	return ft6a;
}

pulse(ft4a){
	printf("FT4A\n");
	apr.f4a = 0;			// 5-4
	apr.ma = 0;			// 7-3
	swap(&apr.mb, &apr.mq);		// 6-3, 6-13
	return ft5;			// 5-4
}

pulse(ft4){
	printf("FT4\n");
	apr.mq = apr.mb;		// 6-13
	apr.f4a = 1;			// 5-4
	apr.mc_rst1_ret = ft4a;
	return mc_rd_rq_pulse;
}

pulse(ft3){
	printf("FT3\n");
	apr.ma = apr.mb & RT;		// 7-3
	swap(&apr.mb, &apr.ar);		// 6-3
	return ft4;			// 5-4
}

pulse(ft1a){
	printf("FT1A\n");
	apr.f1a = 0;			// 5-4
	if(apr.fac2)
		apr.ma = apr.ma+1 & 017;	// 7-1, 7-5
	else
		apr.ma = 0;		// 7-3
	if(!(apr.fc_c_aclt || apr.fc_c_acrt))
		swap(&apr.mb, &apr.ar);	// 6-3
	if(apr.fc_c_aclt)
		apr.mb = apr.mb<<18 & LT | apr.mb>>18 & RT;	// 6-3
	if(apr.fac2)
		return ft4;		// 5-4
	if(apr.fc_c_aclt || apr.fc_c_acrt)
		return ft3;		// 5-4
	return ft5;			// 5-4
}

pulse(ft1){
	printf("FT1\n");
	apr.ma = apr.ir>>5 & 017;	// 7-3
	apr.f1a = 1;			// 5-4
	apr.mc_rst1_ret = ft1a;
	return mc_rd_rq_pulse;
}

pulse(ft0){
	printf("FT0\n");
	if(apr.fac_inh)
		return ft5;		// 5-4
	return ft1;			// 5-4
}

/*
 * Address
 */

pulse(at5){
	printf("AT5\n");
	apr.a_long = 1;			// nowhere to be found :(
	apr.af0 = 1;			// 5-3
	apr.ma = apr.mb & RT;		// 7-3
	apr.ir &= ~037;			// 5-7
	apr.mc_rst1_ret = at0;
	return mc_rd_rq_pulse;
}

pulse(at4){
	printf("AT4\n");
	apr.ar &= ~LT;			// 6-8
	// TODO: what is MC DR SPLIT?
	if(apr.sw_addr_stop || apr.key_mem_stop)
		apr.mc_split_cyc_sync = 1;	// 7-9
	if(apr.ir & 020)
		return at5;		// 5-3
	return ft0;			// 5-4
}

pulse(at3a){
	printf("AT3A\n");
	apr.af3a = 0;			// 5-3
	apr.mb = apr.ar;		// 6-3
	return at4;			// 5-3
}

pulse(at3){
	printf("AT3\n");
	apr.af3 = 0;			// 5-3
	apr.ma = 0;			// 7-3
	apr.af3a = 1;			// 5-3
	apr.art3_ret = at3a;
	return ar_ast1;
}

pulse(at2){
	printf("AT2\n");
	apr.a_long = 1;			// nowhere to be found :(
	apr.ma = apr.ir & 017;		// 7-3
	apr.af3 = 1;			// 5-3
	apr.mc_rst1_ret = at3;
	return mc_rd_rq_pulse;
}

pulse(at1){
	printf("AT1\n");
	apr.ex_uuo_sync = 1;		// 5-13
	if((apr.ir & 017) == 0)
		return at4;		// 5-3
	return at2;			// 5-3
}

pulse(at0){
	printf("AT0\n");
	apr.ar &= ~RT;			// 6-8
	apr.ar |= apr.mb & RT;		// 6-8
	apr.ir |= apr.mb>>18 & 037;	// 5-7
	decodeir();
	apr.ma = 0;			// 7-3
	apr.af0 = 0;			// 5-3
	return pi_sync;			// 8-4
}

/*
 * Instruction
 */

pulse(iat0){
	printf("IAT0\n");
	mr_clr();
	set_pi_cyc(1);
	return it1;
}

pulse(it1a){
	printf("IT1A\n");
	apr.if1a = 0;
	apr.ir |= apr.mb>>18 & 0777740;	// 5-7
	if(apr.ma & 0777760)
		set_key_rim_sbr(0);	// 5-2
	return at0;
}

pulse(it1){
	printf("IT1\n");
	hword n;
	u8 r;
	if(apr.pi_cyc){
		// 7-3
		r = apr.pi_req;
		for(n = 7; !(r & 1); n--, r >>= 1);
		apr.ma = 040 | n<<1;
	}else
		apr.ma = apr.pc;	// 7-3
	if(apr.pi_ov)
		apr.ma = (apr.ma+1)&RT;	// 7-3
	apr.if1a = 1;
	apr.mc_rst1_ret = it1a;
	return mc_rd_rq_pulse;
}

pulse(it0){
	printf("IT0\n");
	apr.ma = 0;
	mr_clr();
	apr.if1a = 1;		// 5-3
	return pi_sync;		// 8-4
}

/*
 * Memory Control
 */

pulse(mc_rs_t1){
	printf("MC RS T1\n");
	set_mc_rd(0);
	if(apr.key_ex_next || apr.key_dep_next)
		apr.mi = apr.mb;	// 7-7
	return apr.mc_rst1_ret;
}

pulse(mc_rs_t0){
	printf("MC RS T0\n");
	apr.mc_stop = 0;
	return mc_rs_t1;
}

pulse(mc_wr_rs){
	printf("MC WR RS\n");
	if(apr.ma == apr.mas)
		apr.mi = apr.mb;	// 7-7
	membus1 = apr.mb;
	membus0 |= MEMBUS_MAI_WR_RS;
	wakemem();
	if(!apr.mc_stop)
		return mc_rs_t0;
	return NULL;
}

pulse(mc_addr_ack){
	printf("MC ADDR ACK\n");
	set_mc_rq(0);
	if(!apr.mc_rd && apr.mc_wr)
		return mc_wr_rs;
	return NULL;
}

pulse(mc_non_exist_rd){
	printf("MC NON EXIST RD\n");
	if(!apr.mc_stop)
		return mc_rs_t0;
	return NULL;
}

pulse(mc_non_exist_mem_rst){
	printf("MC NON EXIST MEM RST\n");
	if(apr.mc_rd){
		/* call directly - no other pulses after it in this case */
		mc_addr_ack();
		return mc_non_exist_rd;
	}else
		return mc_addr_ack();
	return NULL;
}

pulse(mc_non_exist_mem){
	printf("MC NON EXIST MEM\n");
	apr.cpa_non_exist_mem = 1;
	// TODO: IOB PI REQ CPA PIA
	if(!apr.sw_mem_disable)
		return mc_non_exist_mem_rst;
	return NULL;
}

pulse(mc_illeg_address){
	printf("MC ILLEG ADDRESS\n");
	apr.cpa_illeg_op = 1;
	// TODO: IOB PI REQ CPA PIA
	return st7;
}

pulse(mc_rq_pulse){
	printf("MC RQ PULSE\n");
	apr.mc_stop = 0;		// 7-9
	/* catch non-existent memory */
	apr.extpulse |= 4;
	if(calcaddr() == 0 && !apr.ex_inh_rel)
		return mc_illeg_address;
	set_mc_rq(1);
	if(apr.key_mem_stop || apr.ma == apr.mas && apr.sw_addr_stop){
		apr.mc_stop = 1;	// 7-9
		// TODO: what is this? does it make any sense?
		if(apr.key_mem_cont)
			return kt4;
	}
	return NULL;
}

pulse(mc_rdwr_rq_pulse){
	printf("MC RD/RW RQ PULSE\n");
	set_mc_rd(1);			// 7-9
	set_mc_wr(1);			// 7-9
	apr.mb = 0;
	apr.mc_stop_sync = 1;		// 7-9
	return mc_rq_pulse;
}

pulse(mc_rd_rq_pulse){
	printf("MC RD RQ PULSE\n");
	set_mc_rd(1);			// 7-9
	set_mc_wr(0);			// 7-9
	apr.mb = 0;
	return mc_rq_pulse;
}

pulse(mc_split_rd_rq){
	printf("MC SPLIT RD RQ\n");
	return mc_rd_rq_pulse;
}

pulse(mc_wr_rq_pulse){
	printf("MC WR RQ PULSE\n");
	set_mc_rd(0);			// 7-9
	set_mc_wr(1);			// 7-9
	return mc_rq_pulse;
}

pulse(mc_split_wr_rq){
	printf("MC SPLIT WR RQ\n");
	return mc_wr_rq_pulse;
}

pulse(mc_rd_wr_rs_pulse){
	printf("MC RD/WR RS PULSE\n");
	return apr.mc_split_cyc_sync ? mc_split_wr_rq : mc_wr_rs;
}

/*
 * Keys
 */

pulse(key_rd_wr_ret){
	printf("KEY RD WR RET\n");
	apr.key_rd_wr = 0;	// 5-2
//	apr.ex_ill_op = 0;	// ?? not found on 5-13
	return kt4;		// 5-2
}

pulse(key_rd){
	printf("KEY RD\n");
	apr.key_rd_wr = 1;	// 5-2
	apr.mc_rst1_ret = key_rd_wr_ret;
	return mc_rd_rq_pulse;
}

pulse(key_wr){
	printf("KEY WR\n");
	apr.key_rd_wr = 1;	// 5-2
	apr.mb = apr.ar;	// 6-3
	apr.mc_rst1_ret = key_rd_wr_ret;
	return mc_wr_rq_pulse;
}

pulse(key_go){
	printf("KEY GO\n");
	apr.run = 1;
	apr.key_ex_st = 0;
	apr.key_dep_st = 0;
	apr.key_ex_sync = 0;
	apr.key_dep_sync = 0;
	return it0;
}

pulse(kt4){
	printf("KT4\n");
	if(apr.run &&
	   (apr.key_ex || apr.key_ex_next || apr.key_dep || apr.key_dep_next))
		return key_go; // 5-2
	// TODO check repeat switch
	return NULL;
}

pulse(kt3){
	printf("KT3\n");
	if(apr.key_readin || apr.key_start)
		apr.pc = apr.ma;	// 5-12
	if(apr.key_execute && !apr.run){
		apr.mb = apr.ar;	// 6-3
		// TODO: go to KT4 to check repeat (when processor is idle)
		return it1a;		// 5-3
	}
	if(apr.key_ex || apr.key_ex_next)
		return key_rd;		// 5-2
	if(apr.key_dep || apr.key_dep_next)
		return key_wr;		// 5-2
	if(apr.key_start || apr.key_readin || apr.key_inst_cont)
		return key_go;		// 5-4
	return NULL;
}

#define KEY_MANUAL (apr.key_readin || apr.key_start || apr.key_inst_cont ||\
                    apr.key_mem_cont || apr.key_ex || apr.key_dep ||\
                    apr.key_ex_next || apr.key_dep_next || apr.key_execute ||\
                    apr.key_io_reset)

pulse(kt12){
	printf("KT1,2\n");
	if(apr.key_io_reset)
		mr_start();	// 5-2
	if(KEY_MANUAL && !apr.key_mem_cont)
		mr_clr();	// 5-2
	if(!(apr.key_readin || apr.key_inst_cont || apr.key_mem_cont))
		set_key_rim_sbr(0);	// 5-2

	if(apr.key_readin){
		set_key_rim_sbr(1);	// 5-2
		apr.ma = apr.mas;
	}
	if(apr.key_start)
		apr.ma = apr.mas;
	if(apr.key_execute && !apr.run)
		apr.ar = apr.data;
	if(apr.key_ex)
		apr.ma = apr.mas;
	if(apr.key_ex_next)
		apr.ma = (apr.ma+1)&RT;
	if(apr.key_dep){
		apr.ma = apr.mas;
		apr.ar = apr.data;
	}
	if(apr.key_dep_next){
		apr.ma = (apr.ma+1)&RT;
		apr.ar = apr.data;
	}

	if(apr.key_mem_cont && apr.mc_stop)
		return mc_rs_t0;
	if(KEY_MANUAL && apr.mc_stop && apr.mc_stop_sync && !apr.key_mem_cont){
		/* Finish rd/wr which should stop the processor.
		 * Set flag to continue at KT3 */
		apr.extpulse |= 2;
		return mc_wr_rs;
	}
	return kt3;	// 5-2
}

pulse(kt0a){
	printf("KT0A\n");
	apr.key_ex_st = 0;	// 5-1
	apr.key_dep_st = 0;	// 5-1
	apr.key_ex_sync = apr.key_ex || apr.key_ex_next;	// 5-1
	apr.key_dep_sync = apr.key_dep || apr.key_dep_next;	// 5-1
	if(!apr.run || apr.key_mem_cont)
		return kt12;	// 5-2
	return NULL;
}

pulse(kt0){
	printf("KT0\n");
	return kt0a;		// 5-2
}

pulse(key_manual){
	printf("KEY MANUAL\n");
	return kt0;		// 5-2
}

pulse(mai_addr_ack){
	printf("MAI ADDR ACK\n");
	return mc_addr_ack;
}

pulse(mai_rd_rs){
	printf("MAI RD RS\n");
	apr.mb = membus1;
	if(apr.ma == apr.mas)
		apr.mi = apr.mb;	// 7-7
	if(!apr.mc_stop)
		return mc_rs_t0;
	return NULL;
}

void*
aprmain(void *p)
{
	mr_pwr_clr();
	while(apr.sw_power){
		if(apr.extpulse & 1){
			if(apr.nextpulse)
				printf("whaa: cpu wasn't idle\n");
			apr.extpulse &= ~1;
			apr.nextpulse = key_manual;
		}
		if(apr.nextpulse)
			apr.nextpulse = apr.nextpulse();
		else if(membus0 & MEMBUS_MAI_ADDR_ACK){
			membus0 &= ~MEMBUS_MAI_ADDR_ACK;
			apr.extpulse &= ~4;
			apr.nextpulse = mai_addr_ack;
		}else if(membus0 & MEMBUS_MAI_RD_RS){
			membus0 &= ~MEMBUS_MAI_RD_RS;
			apr.nextpulse = mai_rd_rs;
		}else if(apr.extpulse & 2){
			/* continue at KT3 after finishing rd/wr from a stop */
			apr.extpulse &= ~2;
			apr.nextpulse = kt3;
		}else if(apr.extpulse & 4){
			apr.extpulse &= ~4;
			if(apr.mc_rq && !apr.mc_stop)
				apr.nextpulse = mc_non_exist_mem;
		}
	}
	printf("power off\n");
	return NULL;
}