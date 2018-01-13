#pragma once
#include <random>
#include <algorithm>
#include <ctime>

namespace mpi
{
	typedef std::numeric_limits<int> int_limits;
	class random
	{
		typedef std::numeric_limits<int> int_limits;

		public:
			random()         = delete;
			random(random&)  = delete;
			random(random&&) = delete;
			random & operator=(const random&) = delete;

			/// Сгенерировать массив случайных чисел по двум итераторам
			template<typename It>
			static void generate(It range_from, It range_to, 
				int from = int_limits::min(), int to = int_limits::max())
			{
				srand(time(0));
				std::generate(range_from, range_to, std::rand);
			}
	};
}
