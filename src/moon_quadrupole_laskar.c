/** * @file moon_quadrupole_laskar.c
 * @brief   Models Earth's moon as a quadrupole around the Earth interacting with the Sun
 * @author  Aleksandar Rachkov, Dan Tamayo <tamayo.daniel@gmail.com>
 * 
 * @section     LICENSE
 * Copyright (c) 2015 Aleksandar Rachkov, Dan Tamayo, Hanno Rein
 *
 * This file is part of reboundx.
 *
 * reboundx is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * reboundx is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with rebound.  If not, see <http://www.gnu.org/licenses/>.
 *
 * The section after the dollar signs gets built into the documentation by a script.  All lines must start with space * space like below.
 * Tables always must be preceded and followed by a blank line.  See http://docutils.sourceforge.net/docs/user/rst/quickstart.html for a primer on rst.
 * $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
 *
 * $Multipoles$       // Effect category (must be the first non-blank line after dollar signs and between dollar signs to be detected by script).
 *
 * ======================= ===============================================
 * Authors                 A. Rachkov, D. Tamayo
 * Implementation Paper    *In progress*
 * Based on                None
 * C Example               :ref:`c_example_moon_quadrupole_laskar`
 * Python Example          `MoonQuadrupoleLaskar.ipynb <https://github.com/dtamayo/reboundx/blob/master/ipython_examples/MoonQuadrupoleLaskar.ipynb>`_.
 * ======================= ===============================================
 * 
 * Add description here 
 *
 * **Effect Parameters**
 * 
 * None  
 *
 * **Particle Parameters**
 * 
 * ============================ =========== ==================================================================
 * Field (C type)               Required    Description
 * ============================ =========== ==================================================================
 * Aradial (double)             Yes         Normalization for central acceleration.
 * gammaradial (double)         Yes         Power index for central acceleration.
 * ============================ =========== ==================================================================
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include "rebound.h"
#include "reboundx.h"

static void rebx_calculate_force(struct reb_simulation* const sim, const double m_ratio_earthmoon_mql, const double a0_mql, const double a1_mql, const double a2_mql, const double alpha_mql, const double f_mql, const int i){
    struct reb_particle* const particles = sim->particles;
    const struct reb_particle source = sim->particles[0];
    const struct reb_particle p = particles[i];
    const double dx = p.x - source.x;
    const double dy = p.y - source.y;
    const double dz = p.z - source.z;
    const double r2 = dx*dx + dy*dy + dz*dz;
    

    // Calculate A. Put in right value of gamma
    double r_earthmoon_mql = a0_mql*pow((1.0+(a1_mql/alpha_mql)*1.e-9*2.*M_PI*0.+a2_mql*1.e-18*4.*M_PI*M_PI*0.*0.),alpha_mql);
//    double r_earthmoon_mql = a0_mql*pow((1.0+(a1_mql/alpha_mql)*1.e-9*2.*M_PI*sim->t+a2_mql*1.e-18*4.*M_PI*M_PI*sim->t*sim->t),alpha_mql);
    double massratio = 1.0/(m_ratio_earthmoon_mql + 2.0 + 1.0/m_ratio_earthmoon_mql);

    const double A = (-3.0/4.0)*sim->G*source.m*(f_mql)*r_earthmoon_mql*r_earthmoon_mql*massratio;
    const double prefac = A*pow(r2, -5./2.);

    particles[i].ax += prefac*dx;
    particles[i].ay += prefac*dy;
    particles[i].az += prefac*dz;
    particles[0].ax -= p.m/source.m*prefac*dx;
    particles[0].ay -= p.m/source.m*prefac*dy;
    particles[0].az -= p.m/source.m*prefac*dz;
}

void rebx_moon_quadrupole_laskar(struct reb_simulation* const sim, struct rebx_effect* const effect){ 
    const int N_real = sim->N - sim->N_var;
    struct reb_particle* const particles = sim->particles;
    for (int i=1; i<N_real; i++){
        const double* const m_ratio_earthmoon_mql = rebx_get_param_check(&particles[i], "m_ratio_earthmoon_mql", REBX_TYPE_DOUBLE);
        if (m_ratio_earthmoon_mql != NULL){
            const double* const a0_mql = rebx_get_param_check(&particles[i], "a0_mql", REBX_TYPE_DOUBLE);
            if (a0_mql != NULL){
		        const double* const a1_mql = rebx_get_param_check(&particles[i], "a1_mql", REBX_TYPE_DOUBLE);
		        if (a1_mql != NULL){
			        const double* const a2_mql = rebx_get_param_check(&particles[i], "a2_mql", REBX_TYPE_DOUBLE);
			        if (a2_mql != NULL){
				        const double* const alpha_mql = rebx_get_param_check(&particles[i], "alpha_mql", REBX_TYPE_DOUBLE);
				        if (alpha_mql != NULL){
					        const double* const f_mql = rebx_get_param_check(&particles[i], "f_mql", REBX_TYPE_DOUBLE);
				            if (f_mql != NULL){
				                rebx_calculate_force(sim, *m_ratio_earthmoon_mql, *a0_mql, *a1_mql, *a2_mql, *alpha_mql, *f_mql, i); // only calculates force if all parameters set
                            }
                        }
                    }
                }
            }
        }
    }
}

static double rebx_calculate_hamiltonian(struct reb_simulation* const sim, const double m_ratio_earthmoon_mql, const double a0_mql, const double a1_mql, const double a2_mql, const double alpha_mql, const double f_mql, const int i){
    const struct reb_particle* const particles = sim->particles;
    const struct reb_particle source = particles[0];
    const struct reb_particle p = particles[i];
    const double dx = p.x - source.x;
    const double dy = p.y - source.y;
    const double dz = p.z - source.z;
    const double r2 = dx*dx + dy*dy + dz*dz;

    double r_earthmoon_mql = a0_mql*pow((1.0+(a1_mql/alpha_mql)*1.e-9*2.*M_PI*0.+a2_mql*1.e-18*4.*M_PI*M_PI*0.*0.),alpha_mql);
//    double r_earthmoon_mql = a0_mql*pow((1.0+(a1_mql/alpha_mql)*1.e-9*2.*M_PI*sim->t+a2_mql*1.e-18*4.*M_PI*M_PI*sim->t*sim->t),alpha_mql);
    double massratio = 1.0/(m_ratio_earthmoon_mql + 2.0 + 1.0/m_ratio_earthmoon_mql);

    const double A = (-1.0/4.0)*sim->G*source.m*(f_mql)*r_earthmoon_mql*r_earthmoon_mql*massratio;
    double H = p.m*A*pow(r2, -3./2.); // figure this out
   
    return H;
}

double rebx_moon_quadrupole_laskar_hamiltonian(struct reb_simulation* const sim){ 
    const int N_real = sim->N - sim->N_var;
    struct reb_particle* const particles = sim->particles;
    double Htot = 0.;
    for (int i=1; i<N_real; i++){
        // copy paste the chain of if statements from above
        const double* const m_ratio_earthmoon_mql = rebx_get_param_check(&particles[i], "m_ratio_earthmoon_mql", REBX_TYPE_DOUBLE);
        if (m_ratio_earthmoon_mql != NULL){
            const double* const a0_mql = rebx_get_param_check(&particles[i], "a0_mql", REBX_TYPE_DOUBLE);
            if (a0_mql != NULL){
                const double* const a1_mql = rebx_get_param_check(&particles[i], "a1_mql", REBX_TYPE_DOUBLE);
                if (a1_mql != NULL){
                    const double* const a2_mql = rebx_get_param_check(&particles[i], "a2_mql", REBX_TYPE_DOUBLE);
                    if (a2_mql != NULL){
                        const double* const alpha_mql = rebx_get_param_check(&particles[i], "alpha_mql", REBX_TYPE_DOUBLE);
                        if (alpha_mql != NULL){
                            const double* const f_mql = rebx_get_param_check(&particles[i], "f_mql", REBX_TYPE_DOUBLE);
                                if (f_mql != NULL){
                                    Htot += rebx_calculate_hamiltonian(sim, *m_ratio_earthmoon_mql, *a0_mql, *a1_mql, *a2_mql, *alpha_mql, *f_mql, i);
                            }
                        }
                    }
                }
            }
        }
    }
    return Htot;
}