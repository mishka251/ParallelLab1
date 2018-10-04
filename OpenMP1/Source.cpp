#include<iostream>
#include<stdlib.h>
#include <chrono>



using namespace std;


void task1()
{
#pragma omp parallel
	cout << "Hello world" << endl;
}

void task2()
{
	const int N = 100000;

	auto start = chrono::steady_clock::now();

#pragma omp parallel for schedule(static)
	for (int i = 0; i < N; i++)
		cout << i << endl;

	auto end = chrono::steady_clock::now();
	auto mic_s1 = std::chrono::duration_cast<chrono::microseconds>(end - start);
	//cout << "time " << mic_s.count() << " ms" << endl;

	start = chrono::steady_clock::now();
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < N; i++)
		cout << i << endl;
	end = chrono::steady_clock::now();
	auto mic_s2 = std::chrono::duration_cast<chrono::microseconds>(end - start);
	//cout << "time " << mic_s.count() << " ms" << endl;

	start = chrono::steady_clock::now();

#pragma omp parallel for schedule(dynamic, 10)
	for (int i = 0; i < N; i++)
		cout << i << endl;
	end = chrono::steady_clock::now();
	auto mic_s3 = std::chrono::duration_cast<chrono::microseconds>(end - start);
	//cout << "time " << mic_s.count() << " ms" << endl;

	start = chrono::steady_clock::now();

#pragma omp parallel for schedule(guided)
	for (int i = 0; i < N; i++)
		cout << i << endl;

	end = chrono::steady_clock::now();
	auto mic_s4 = std::chrono::duration_cast<chrono::microseconds>(end - start);

	cout << "time1 " << mic_s1.count() << " ms" << endl;
	cout << "time2 " << mic_s2.count() << " ms" << endl;
	cout << "time3 " << mic_s3.count() << " ms" << endl;
	cout << "time4 " << mic_s4.count() << " ms" << endl;
}


void task3()
{
	const int N = 100000;
	int *arr = new int[N];
	int sum = 0;

#pragma omp parallel for
	for (int i = 0; i < N; i++)
		arr[i] = rand() % 50;

	auto start = chrono::steady_clock::now();
#pragma omp parallel for
	for (int i = 0; i < N; i++)
#pragma omp critical(sum)
		sum += arr[i];
	auto end = chrono::steady_clock::now();
	auto mic_s1 = chrono::duration_cast<chrono::microseconds> (end - start);
	cout << "sum1 = " << sum << endl;
	sum = 0;

	start = chrono::steady_clock::now();
#pragma omp parallel for reduction(+:sum)
	for (int i = 0; i < N; i++)
		sum += arr[i];


	end = chrono::steady_clock::now();
	auto mic_s2 = chrono::duration_cast<chrono::microseconds> (end - start);

	cout << "sum2 = " << sum << endl;
	sum = 0;

	start = chrono::steady_clock::now();
#pragma omp parallel for 
	for (int i = 0; i < N; i++)
#pragma omp atomic
		sum += arr[i];


	end = chrono::steady_clock::now();
	auto mic_s3 = chrono::duration_cast<chrono::microseconds> (end - start);


	cout << "sum3 = " << sum << endl;
	cout << "time 1 =" << mic_s1.count() << endl;
	cout << "time 2 =" << mic_s2.count() << endl;
	cout << "time 3 =" << mic_s3.count() << endl;

}

void task4()
{
	int value = 100;
#pragma omp parallel 
	{
#pragma omp atomic
		value++;
#pragma omp barrier
#pragma omp critical (cout)
		{
			std::cout << value << std::endl;
		}
	}
}


void task5()
{
	const int N = 100, M = 5;
	int A[N][N], A1[N][N];

	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
		{
			A[i][j] = rand() % 30;
			A1[i][j] = A[i][j];
		}



	/*for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
			cout << A[i][j] << "  ";
		cout << endl;
	}

	cout << endl << endl << endl;
*/


	auto start = chrono::steady_clock::now();

	for (int i = 2; i < N; i++)
		for (int j = 2; j < N; j++)
			A[i][j] = A[i - 2][j] + A[i][j - 2];

	auto end = chrono::steady_clock::now();
	auto mic_s1 = chrono::duration_cast<chrono::microseconds>(end - start);

	start = chrono::steady_clock::now();
#pragma omp parallel for num_threads(2) //ordered
	for (int i = 2; i < N; i++)
	{
//#pragma omp ordered
	//	{
#pragma omp parallel for num_threads(2)
			for (int j = 2; j < N; j++)
				A1[i][j] = A1[i - 2][j] + A1[i][j - 2];
		//}
	}

	end = chrono::steady_clock::now();
	auto mic_s2 = chrono::duration_cast<chrono::microseconds>(end - start);

	/*for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
			cout << A[i][j] << "  ";
		cout << endl;
	}

	cout << endl << endl << endl;

	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
			cout << A1[i][j] << "  ";
		cout << endl;
	}
	cout << endl << endl << endl;
*/
	bool correct = true;
#pragma omp parallel for reduction(&&:correct)  num_threads(2)
	for (int i = 0; i < N; i++)
	{
		bool tmp = true;
#pragma omp parallel for reduction(&&:tmp)  num_threads(2)
		for (int j = 0; j < N; j++)
			tmp = tmp && (A[i][j] == A1[i][j]);
		correct = correct && tmp;

	}

	cout << "correct = " << correct << endl;
	cout << "time 1 =" << mic_s1.count() << endl;
	cout << "time 2 =" << mic_s2.count() << endl;
}


int main()
{

	task5();

	system("pause");
	return 0;
}