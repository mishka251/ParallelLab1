#include<iostream>
#include<stdlib.h>
#include <chrono>
#include<functional>
#include<string>

using namespace std;


std::function<void(int i, int N)> functions[] =
{
	[](int i, int n) {std::cout << i << endl; },
	[](int i, int n) {if (i % 2 == 1) for (int j = 0; j < 1000; j++) i -= j; std::cout << i << endl;  },
	[](int i, int n) {if (i < n / 2) for (int j = 0; j < 1000; j++) i -= j; std::cout << i << endl; }
};


void task1()
{
#pragma omp parallel
	cout << "Hello world" << endl;
}

void task2()
{
	const int N = 1000;

	/*const*/ int cnt = 10;

	int time_mat[4][3];
	string names[] = { "static", "dynamic", "dynamic 10", "guided" };


	for (int func = 0; func < 3; func++)
	{

		auto start = chrono::steady_clock::now();

#pragma omp parallel for schedule(static)
		for (int i = 0; i < N; i++)
			functions[func](i, N);

		auto end = chrono::steady_clock::now();
		auto mic_s1 = std::chrono::duration_cast<chrono::microseconds>(end - start);
		//cout << "time " << mic_s.count() << " ms" << endl;

		start = chrono::steady_clock::now();
#pragma omp parallel for schedule(dynamic)
		for (int i = 0; i < N; i++)
			functions[func](i, N);

		end = chrono::steady_clock::now();
		auto mic_s2 = std::chrono::duration_cast<chrono::microseconds>(end - start);
		//cout << "time " << mic_s.count() << " ms" << endl;

		start = chrono::steady_clock::now();

#pragma omp parallel for schedule(dynamic, cnt)
		for (int i = 0; i < N; i++)
			functions[func](i, N);
		end = chrono::steady_clock::now();
		auto mic_s3 = std::chrono::duration_cast<chrono::microseconds>(end - start);
		//cout << "time " << mic_s.count() << " ms" << endl;

		start = chrono::steady_clock::now();

#pragma omp parallel for schedule(guided)
		for (int i = 0; i < N; i++)
			functions[func](i, N);

		end = chrono::steady_clock::now();
		auto mic_s4 = std::chrono::duration_cast<chrono::microseconds>(end - start);

		time_mat[0][func] = mic_s1.count();
		time_mat[1][func] = mic_s2.count();
		time_mat[2][func] = mic_s3.count();
		time_mat[3][func] = mic_s4.count();
	}

	cout << "название     равномерное    нечетное      первая_половина"<<endl;
	for (int i = 0; i < 4; i++)
	{
		cout << names[i] << "       ";
		for (int j = 0; j < 3; j++)
			cout << "   " << time_mat[i][j] << "     ";
		cout<< endl;
	}
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
	const int N = 50, M = 5;
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

	task2();

	system("pause");
	return 0;
}