/***********************************************************************************/
/*  Copyright 2009 WSL Institute for Snow and Avalanche Research    SLF-DAVOS      */
/***********************************************************************************/
/* This file is part of MeteoIO.
    MeteoIO is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    MeteoIO is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with MeteoIO.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <meteoio/meteofilters/FilterMax.h>

using namespace std;

namespace mio {

FilterMax::FilterMax(const std::vector<std::string>& vec_args, const std::string& name)
          : FilterBlock(name), max_val(0.), max_soft(0.), is_soft(true)
{
	parse_args(vec_args);
	properties.stage = ProcessingProperties::both; //for the rest: default values
}

void FilterMax::process(const unsigned int& param, const std::vector<MeteoData>& ivec,
                        std::vector<MeteoData>& ovec)
{
	ovec = ivec;
	for (size_t ii=0; ii<ovec.size(); ii++){
		double& tmp = ovec[ii](param);
		if (tmp == IOUtils::nodata) continue; //preserve nodata values

		if (tmp > max_val){
			if (is_soft){
				tmp = max_soft;
			} else {
				tmp = IOUtils::nodata;
			}
		}
	}
}


void FilterMax::parse_args(std::vector<std::string> vec_args) {
	vector<double> filter_args;

	is_soft = false;
	if (vec_args.size() > 1){
		is_soft = ProcessingBlock::is_soft(vec_args);
	}

	convert_args(1, 2, vec_args, filter_args);

	if (filter_args.size() > 2)
		throw InvalidArgumentException("Wrong number of arguments for filter " + getName(), AT);

	max_val = filter_args[0];

	if (filter_args.size() == 2){
		max_soft = filter_args[1];
	} else {
		max_soft = max_val;
	}
}

} //end namespace
