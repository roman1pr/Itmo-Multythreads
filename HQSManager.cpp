#include "HQSManager.h"
#include "random.h"
using namespace mpi;
//#define debug

template <typename T>
double HQSManager<T>::experiment(shArray<T>& data, int numbExp)
{
	double min_time = int_limits::max();
	double max_time = int_limits::min();
	double avg_time = 0;
	double time = 0;

	min_time = max_time = avg_time = mpi::HQSManager<int>::sortWithTime(data);

#ifdef debug 
	for (int i = 0; i < numbExp; i++)
	{
		std::cout << "st sort 2 :" << std::endl;
		time = mpi::HQSManager<int>::sortWithTime(data);
		std::cout << "end sort 2 :" << std::endl;
		avg_time += time;
		if (max_time < time) max_time = time;
		if (min_time > time) min_time = time;
	}
	std::cout << "max time:" << max_time << std::endl;
	std::cout << "min time:" << min_time << std::endl;
	std::cout << "average time:" << avg_time << std::endl;
#endif

	return avg_time;
}

template <typename T>
double HQSManager<T>::sortWithTime(shArray<T>& data)
{
	double t1, t2, dt;
	
	mpi::barrier();
	t1 = MPI_Wtime();

	mpi::HQSManager<int>::sortHQS(data);

	mpi::barrier();
	t2 = MPI_Wtime();
	dt = t2 - t1;

	return dt;
}

template <class T>
void HQSManager<T>::sortHQS(shArray<T>& data)
{
	auto slice = split(data);
	qsortpart(slice);
	data = collect(slice);
}

template <typename T>
shArray<T> HQSManager<T>::split(shArray<T>& data)
{
	T* raw = data.get();
	auto slices = slice(raw, raw + data.size(), mpi::getSize(MPI_COMM_WORLD));
	auto groups = distance(slices);
	return mpi::scatter(data, groups, 0);
}

template <typename T>
T HQSManager<T>::select_pivot(shArray<T>& data)
{
	std::sort(std::begin(data), std::end(data));
	return data[data.size() / 2];
}

template <typename T>
void HQSManager<T>::diffusion(T& pivot, int iteration)
{
	int rank = mpi::getRank(MPI_COMM_WORLD),
	    size = mpi::getSize(MPI_COMM_WORLD);
	int root = (iteration == log2(size))
		           ? 0
		           : ((rank >> iteration) << iteration);
	int relative = rank - root;

	for (auto k = 0; k < iteration; k++)
	{
		if (relative < (0x1 << k))
		{
			// Отправка опорного элемента
			mpi::send(pivot, rank + (0x1 << k), 666);
		}
		else if (relative < (0x1 << (k + 1)))
		{
			// Получение опорного элемента
			pivot = mpi::receive<T>(rank - (0x1 << k), 666);
		}
	}
}

template <typename T>
void HQSManager<T>::partition(const T pivot, const shArray<T>& data, shArray<T>& lowPart,
	shArray<T>& highPart)
{
	auto high = 0,
	     low = 0;
	// Считаем размеры для массивов low / high
	for (auto i = 0; i < data.size(); i++)
		(data[i] < pivot) ? low++ : high++;
	// Заново инициализируем массивы
	lowPart.reallocate(low);
	highPart.reallocate(high);
	// Записываем значения в массивы
	for (auto i = 0, h = 0, l = 0; i < data.size(); i++)
		(data[i] < pivot)
			? lowPart[l++] = data[i]
			: highPart[h++] = data[i];
}

template <typename T>
void HQSManager<T>::exchange(shArray<T>& data, const int iteration, const bool isHighPart)
{
	// Определяем ранг текущего процесса и соседа
	// для отправки данных
	int rank = mpi::getRank(MPI_COMM_WORLD),
	    neighbor = rank ^ (0x1 << iteration - 1),
	    size = data.size(), nsize = 0;
	// Обмен массивами
	data = mpi::sendreceive(data, neighbor, neighbor, 666);
}

template <typename T>
void HQSManager<T>::merge(shArray<T>& result, const shArray<T>& one, const shArray<T>& two)
{
	result.reallocate(one.size() + two.size());
	// итератор оригинального массива
	size_t k = 0;

	for (size_t i = 0; i < one.size(); i++)
		result[k++] = one[i];

	for (size_t i = 0; i < two.size(); i++)
		result[k++] = two[i];

	// Сортировка полученного массива
	std::sort(std::begin(result), std::end(result));
}

template <typename T>
void HQSManager<T>::qsortpart(shArray<T>& slice)
{
	auto rank = mpi::getRank(MPI_COMM_WORLD);
	auto size = mpi::getSize(MPI_COMM_WORLD);

	// Размерность гиперкуба
	int dim = log2(size);
	int slen = slice.size();

	T pivot = 0;
	// Массивы значений > и < чем опорный
	shArray<T> highPart{}, lowPart{};
	
	for (auto i = dim; i > 0; i--)
	{
		if (slen != 0)
		{
			pivot = select_pivot(slice);
		}

		// Рассылка опорного элемента соседним процессам
		diffusion(pivot, i);

		// Разбивка на части больше и меньше опорного элемента
		partition(pivot, slice, lowPart, highPart);

		// Обмен частями 
		if (!(rank >> (i - 1) & 0x1))
		{
			exchange(highPart, i, true);
		}
		else
		{
			exchange(lowPart, i, false);
		}

		// Слияние частей 
		merge(slice, highPart, lowPart);
	}
}

template <typename T>
shArray<T> HQSManager<T>::collect(const shArray<T>& slice)
{
	return mpi::gather(slice, 0);
}

template <typename T>
template <typename It>
vector<pair<It, It>> HQSManager<T>::slice(It range_from, It range_to, const ptrdiff_t num)
{
	using diff_t = ptrdiff_t;
	// Кол-во и размер слайсов 
	const diff_t total{std::distance(range_from, range_to)};
	const diff_t portion{total / num};
	// Результирующий вектор
	vector<pair<It, It>> slices(num);
	// Указатель на конец слайса
	It portion_end{range_from};
	// Использование алгоритма 'generate' для создания слайсов
	std::generate(std::begin(slices), std::end(slices), [&portion_end, portion]
	{
		// Указатель на начало текущего слайса
		It portion_start{portion_end};
		// Обработка слайса
		std::advance(portion_end, portion);
		return std::make_pair(portion_start, portion_end);
	});
	// Указатель на конец для последней порции всегда должен указывать на range_to
	slices.back().second = range_to;
	return slices;
}

// Группирует массив слайсов в целочисленный вектор
// размера slices.size(), где каждый элемент соответствует
// размеру i-го слайса
template <typename T>
template <typename It>
vector<int> HQSManager<T>::distance(vector<pair<It, It>>& slices)
{
	vector<int> distances{};
	distances.reserve(slices.size());
	for (const auto& slice : slices)
		distances.push_back(std::distance(slice.first, slice.second));
	return distances;
}

template class HQSManager<int>;
