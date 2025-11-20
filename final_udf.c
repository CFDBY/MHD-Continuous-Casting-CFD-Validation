#include "udf.h"

DEFINE_SOURCE(lorentz_source_zhang_method, c, t, dS, eqn)
{
    /* --- MAKALEDEN ALINAN PARAMETRELER --- */
    real B_max = 0.028;       // Magnetic Induction [Tesla] (Ref: Table III)
    real sigma = 3.6e6;       // Conductivity [S/m] (Ref: Table IV)
    real f = 50.0;            // Frequency [Hz]
    
    /* Açısal Hız (Omega) Hesabı: 2 * pi * f = 314.16 rad/s */
    /* BU CARPAN EKSIK OLDUGU ICIN HIZ DUSUK CIKIYORDU */
    real omega_B = 2.0 * M_PI * f; 

    /* --- 1. KOORDINATLAR (Yatırılmış Mesh İçin) --- */
    real xc[ND_ND];
    C_CENTROID(xc, c, t);
    
    /* Mesh yatırıldığı için: 
       xc[0] = Boyuna Eksen (Axial)
       xc[1] = Yarıçap (Radial) -> r */
    real r = xc[1]; 

    /* Swirl Velocity (Teğetsel Hız) */
    real u_theta = C_W(c, t); 

    /* Sıfıra bölünme koruması */
    if (r < 1.0e-5) r = 1.0e-5;

    /* --- 2. SLIP FACTOR (Kayma Faktörü) --- */
    /* Sıvı hızlandıkça kuvvet azalır */
    real slip_factor = 1.0 - (u_theta / (omega_B * r));

    /* --- 3. SOFT START (Ramping) --- */
    /* İlk 0.1 saniyede kuvveti yavaşça devreye al */
    real current_time = CURRENT_TIME;
    real ramp_time = 0.1; 
    real time_factor = 1.0;
    
    if (current_time < ramp_time)
        time_factor = current_time / ramp_time;

    /* --- 4. KUVVET HESABI (DUZELTILMIS) --- */
    /* Formül: F = 0.5 * sigma * omega_B * B^2 * r * slip_factor */
    /* omega_B çarpanı eklendi! */
    
    real F_theta = 0.5 * sigma * omega_B * B_max * B_max * r * slip_factor * time_factor;

    /* --- 5. TUREV (dS) --- */
    /* dF/du = -0.5 * sigma * B^2  (Matematiksel sadeleşme sonrası) */
    /* Negatif türev çözücü kararlılığını artırır */
    
    dS[eqn] = -0.5 * sigma * B_max * B_max * time_factor;

    return F_theta;
}