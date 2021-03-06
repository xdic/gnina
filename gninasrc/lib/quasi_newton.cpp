/*

 Copyright (c) 2006-2010, The Scripps Research Institute

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 Author: Dr. Oleg Trott <ot14@columbia.edu>,
 The Olson Lab,
 The Scripps Research Institute

 */

#include "quasi_newton.h"
#include "conf_gpu.h"
#include "bfgs.h"
#include "non_cache_gpu.h"


struct quasi_newton_aux {
	model* m;
	const precalculate* p;
	const igrid* ig;
	const vec v;
	const grid* user_grid;
	quasi_newton_aux(model* m_,const precalculate* p_,const igrid* ig_,
			const vec& v_,const grid* user_grid_) :
			m(m_), p(p_), ig(ig_), v(v_), user_grid(user_grid_){
	}

	fl operator()(const conf& c,change& g){
		return m->eval_deriv(*p, *ig, v, c, g, *user_grid);
	}
};

struct quasi_newton_aux_gpu {
	model* m;
	const precalculate* p;
	const non_cache_gpu* ig;
	const vec v;
	const grid* user_grid;
	quasi_newton_aux_gpu(model* m_,const precalculate* p_,const non_cache_gpu* ig_,
			const vec& v_,const grid* user_grid_) :
			m(m_), p(p_), ig(ig_), v(v_), user_grid(user_grid_){
		m->copy_to_gpu();
	}

	~quasi_newton_aux_gpu() {
		m->copy_from_gpu();
	}

	fl operator()(const conf_gpu& c,change_gpu& g){
		return m->eval_deriv_gpu(*p, *ig, v, c, g, *user_grid);
	}
};


void quasi_newton::operator()(model& m,const precalculate& p,const igrid& ig,
		output_type& out,change& g,const vec& v,const grid& user_grid) const{ // g must have correct size

	const non_cache_gpu* gpu = dynamic_cast<const non_cache_gpu*>(&ig);
	if(gpu) {
		m.initialize_gpu();
		assert(m.gpu_initialized());
		quasi_newton_aux_gpu aux(&m, &p, gpu, v, &user_grid);
		change_gpu gchange(g);
		conf_gpu gconf(out.c);
		fl res = bfgs(aux, gconf, gchange, average_required_improvement, params);
		gconf.set_cpu(out.c);
		out.e = res;
	} else {
		quasi_newton_aux aux(&m, &p, &ig, v, &user_grid);
		fl res = bfgs(aux, out.c, g, average_required_improvement, params);
		out.e = res;
	}
}

