#include<iostream>
#include<stdlib.h>
#include <chrono>
#include<functional>
#include<string>
#include<locale.h>
#include<windows.h>
#include<omp.h>

//#pragma comment(linker, "/STACK:1073741824")

using namespace std;


std::function<void(int i, int N)> functions[] =
{
	[](int i, int n) {Sleep(10); },//10мс
	[](int i, int n) {if (i % 2 == 1) Sleep(10); else Sleep(1);  },
	[](int i, int n) {if (i < n / 2) Sleep(10); else Sleep(1); }
};


void task1()
{
#pragma omp parallel
	cout << "Hello world" << endl;
}

void task2()
{
	const int N = 5000;

	int time_mat[4][3];
	string names[] = { "static", "dynamic", "dynamic 8", "guided" };


	for (int func = 0; func < 3; func++)
	{
		cout << "f" << (func + 1) << endl;
		auto start = omp_get_wtime();

		cout << "loop1" << endl;
#pragma omp parallel for schedule(static)
		for (int i = 0; i < N; i++)
			functions[func](i, N);

		auto end = omp_get_wtime();
		auto mic_s1 = end - start;

		cout << "loop2" << endl;
		start = omp_get_wtime();
#pragma omp parallel for schedule(dynamic)
		for (int i = 0; i < N; i++)
			functions[func](i, N);

		end = omp_get_wtime();
		auto mic_s2 = end - start;

		cout << "loop3" << endl;
		start = omp_get_wtime();

#pragma omp parallel for schedule(dynamic, 8)
		for (int i = 0; i < N; i++)
			functions[func](i, N);
		end = omp_get_wtime();
		auto mic_s3 = end - start;


		start = omp_get_wtime();
		cout << "loop4" << endl;
#pragma omp parallel for schedule(guided)
		for (int i = 0; i < N; i++)
			functions[func](i, N);

		end = omp_get_wtime();
		auto mic_s4 = end - start;

		time_mat[0][func] = mic_s1;
		time_mat[1][func] = mic_s2;
		time_mat[2][func] = mic_s3;
		time_mat[3][func] = mic_s4;
	}

	cout << "название     равномерное    нечетное      первая_половина" << endl;
	for (int i = 0; i < 4; i++)
	{
		cout << names[i] << "       ";
		for (int j = 0; j < 3; j++)
			cout << "   " << time_mat[i][j] << "     ";
		cout << endl;
	}
}


void task3()
{
	const int N = 100000;
	int *arr = new int[N];
	int sum = 0;

#pragma omp parallel for
	for (int i = 0; i < N; i++)
		arr[i] = rand() % 10;

	auto start = omp_get_wtime();
#pragma omp parallel for
	for (int i = 0; i < N; i++)
#pragma omp critical(sum)
		sum += arr[i];
	auto end = omp_get_wtime();
	auto mic_s1 = end - start;
	cout << "сумма с omp critical = " << sum << endl;

	sum = 0;

	start = omp_get_wtime();
#pragma omp parallel for reduction(+:sum)
	for (int i = 0; i < N; i++)
		sum += arr[i];


	end = omp_get_wtime();
	auto mic_s2 = end - start;
	cout << "сумма с reduction =" << sum << endl;

	sum = 0;
	start = omp_get_wtime();
#pragma omp parallel for 
	for (int i = 0; i < N; i++)
#pragma omp atomic
		sum += arr[i];


	end = omp_get_wtime();
	auto mic_s3 = end - start;


	cout << "сумма с  atomic = " << sum << endl;
	cout << "время с  critical =" << mic_s1 << endl;
	cout << "время с reduction =" << mic_s2 << endl;
	cout << "время с  atomic =" << mic_s3 << endl;

}

void printMatrix(const int N, char **A)
{
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
			cout << A[i][j] << "  ";
		cout << endl;
	}

	cout << endl << endl << endl;
}


void sortRows(char**ar, int N)
{

	int k = omp_get_num_threads();
	for (int i = omp_get_thread_num(); i < N; i += k) {
		for (int x = 0; x < N - 1; ++x)
			for (int y = x + 1; y < N; ++y)
				if (ar[i][x] > ar[i][y]) {
					int swap = ar[i][x];
					ar[i][x] = ar[i][y];
					ar[i][y] = swap;
				}
	}

}
void sortCols(char**ar, int N)
{
	int k = omp_get_num_threads();

	for (int i = omp_get_thread_num(); i < N; i += k) {
		for (int x = 0; x < N - 1; ++x)
			for (int y = x + 1; y < N; ++y)
				if (ar[x][i] > ar[y][i]) {
					int swap = ar[x][i];
					ar[x][i] = ar[y][i];
					ar[y][i] = swap;
				}
	}
}


void func4() {
	int N = 10;
	char** ar = new char*[N], **ar1 = new char*[N];
	for (int i = 0; i < N; ++i) {
		ar[i] = new char[N];
		ar1[i] = new char[N];
		for (int j = 0; j < N; ++j) {
			ar[i][j] = 'a'+rand() %('z'-'a') ;
			ar1[i][j] = ar[i][j];
		}
	}
	cout << "Исходная матрица" << endl;
	printMatrix(N, ar);

#pragma omp parallel
	{
		sortRows(ar, N);
		#pragma omp barrier
		sortCols(ar, N);
	}

#pragma omp parallel
	{
		sortRows(ar1, N);
//#pragma omp barrier
		sortCols(ar1, N);
	}

	cout << "с барьером" << endl;
	printMatrix(N, ar);
	cout << "без барьера" << endl;
	printMatrix(N, ar1);
}


bool checkMatrixes(char**A1, char**A2, int N)
{
	bool correct = true;
#pragma omp parallel for reduction(&&:correct)  
	for (int i = 0; i < N; i++)
	{
		bool tmp = true;
#pragma omp parallel for reduction(&&:tmp) 
		for (int j = 0; j < N; j++)
			tmp = tmp && (A1[i][j] == A2[i][j]);
		correct = correct && tmp;

	}
	return correct;
}

void mySection(char** A, int N, int i)
{
	for (int j = 2; j < N; j += 2)
		if (j == N - 1)
			A[i][j] = A[i - 2][j] + A[i][j - 2];
		else
		{
#pragma omp parallel num_threads(2)
			{
#pragma omp sections
				{
#pragma omp section
					{
						A[i][j] = A[i - 2][j] + A[i][j - 2];
					}

#pragma omp section
					{
						A[i][j+1] = A[i - 2][j+1] + A[i][j - 1];
					}
				}
			}
		}

}


void mySection1(char** A, int N, int i)
{
	for (int j = 2; j < N; j ++)
			A[i][j] = A[i - 2][j] + A[i][j - 2];
}

void task5()
{
	const int N = 2000;
	char **A, **A1, **A2;

	A = new char*[N];
	A1 = new char*[N];
	A2 = new char*[N];

	for (int i = 0; i < N; i++)
	{
		A[i] = new char[N];
		A1[i] = new char[N];
		A2[i] = new char[N];
		for (int j = 0; j < N; j++)
		{
			A[i][j] = 9 - rand() % 10;//INT_MAX - rand() % (INT_MAX / 2);
			A1[i][j] = A[i][j];
			A2[i][j] = A[i][j];
		}
	}
	//printMatrix(N, A);

	auto start = omp_get_wtime();

	for (int i = 2; i < N; i++)
		for (int j = 2; j < N; j++)
			A[i][j] = A[i - 2][j] + A[i][j - 2];

	auto end = omp_get_wtime();

	auto mic_s1 = end - start;
	start = omp_get_wtime();
	for (int i = 2; i < N; i += 2)
	{
		if (i == N - 1)
		{
			mySection1(A1, N, i);
		}
		else
		{
#pragma omp parallel num_threads(2)
#pragma omp  sections
			{
#pragma omp section
				{
					mySection1(A1, N, i);
				}
#pragma omp section
				{
					mySection1(A1, N, i+1);
				}
			}
		}
	}
	end = omp_get_wtime();
	auto mic_s2 = end - start;

	start = omp_get_wtime();
	for (int i = 2; i < N; i += 2)
	{
		if (i == N - 1)
		{
			mySection(A2, N, i);
		}
		else
		{
#pragma omp parallel num_threads(2)
#pragma omp  sections
			{
#pragma omp section
				{
					mySection(A2, N, i);
				}
#pragma omp section
				{
					mySection(A2, N, i + 1);
				}
			}
		}
	}
	end = omp_get_wtime();
	auto mic_s3 = end - start;

	bool correct = checkMatrixes(A, A1, N);
	bool correct2 = checkMatrixes(A, A2, N);

	/*
		printMatrix(N, A);
		printMatrix(N, A1);
		printMatrix(N, A2);
	*/

	cout << "вермя без распараллеливания =" << mic_s1 << endl;
	cout << "параллельное время (распараллелн цикл по i)= " << mic_s2 << endl;
	cout << "параллельное время(распараллелены оба цикла) = " << mic_s3 << endl;

	cout << "результат без paralell == результат параллельного по i = " << correct << endl;
	cout << "результат без paralell == результат параллельного по обоим циклам =  = " << correct2 << endl;


	for (int i = 0; i < N; i++)
	{
		delete[] A[i];
		delete[] A1[i];
		delete[] A2[i];
	}
	delete[] A; delete[] A1; delete[] A2;
}

int main()
{
	setlocale(LC_ALL, "Rus");
	srand(unsigned(time(0)));
	cout << "Задание 1" << endl;
	task1();
	cout << "Задание 2" << endl;
	task2();
	cout << "Задание 3" << endl;
	task3();
	cout << "Задание 4" << endl;
	func4();
	cout << "Задание 5" << endl;
	task5();
	system("pause");
	return 0;
}
