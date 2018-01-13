#pragma once
#include <algorithm>
#include <vector>
#include <memory>
#include <functional>

#include "shArray.h"

namespace mpi {
	using std::vector;
	using std::shared_ptr;
	using std::pair;

	template<typename T> class HQSManager {

	public:
		// запуск сортировки несколько раз, для чистоты экспиремента
		static double experiment(shArray<T>& data, int numbExp);
		// непосредственно сортировка
		static void sortHQS(shArray<T>& data);

	private:
		//вызов sortHQS с временем выполнения
		static double sortWithTime(shArray<T>& data);

		// Разбиение массива на N частей и отправка каждой части собственному процессу
		static shArray<T> split(shArray<T>& data);

		// Выбор опорного элемента
		static T select_pivot(shArray<T>& data);

		// Отправка и получение опорного элемента
		static void diffusion(T& pivot, int iteration);

		// Разделение массива на две части highpart и lowpart
		static void partition(const T pivot, const shArray<T>& data,
		                      shArray<T>& lowPart, shArray<T>& highPart);

		// Обмен данными с соседним процессом не текущей итерации
		static void exchange(shArray<T>& data, const int iteration, const bool isHighPart);

		// Слияние двух массивов в один
		static void merge(shArray<T>& result, const shArray<T>& one, const shArray<T>& two);

		// Итеративная часть алгоритма параллельной сортировки
		static void qsortpart(shArray<T>& slice);

		// Сбор собственных частей массива в корневой процесс
		static shArray<T> collect(const shArray<T>& slice);

		// Разрезает массив на N групп
		template<typename It>
		static vector<pair<It, It>> slice(It range_from, It range_to, const ptrdiff_t num);

		// Группирует массив отрезков в целочисленный вектор
		template<typename It>
		static vector<int> distance(vector<pair<It, It>>& slices);
	};


}
