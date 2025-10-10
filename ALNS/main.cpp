#include<iostream>
#include <fstream>
#include <sstream>
#include "DataDefinition.h"
#include<iomanip>
#include<string>
#include<cmath>
#include"stdlib.h"
#include <windows.h>
#include<time.h>
#include<ctime>
#include <vector>

using namespace std;

#define Random(x) (rand() % x)+1 
#define min(x,y) ((x)<(y)?(x):(y))
#define max(x,y) ((x)>(y)?(x):(y))

string Transform(int a)
{// Function: Convert numbers to words
	string res;
	stringstream ss;
	ss << a;
	ss >> res;
	return res;
}

void Read_Data(int D_Num, int S_Num, int C_Num, int H_Num, int XH, First_Node*& Node, Second_Cust**& Cust)																	  
{//read data
	ifstream infile("cust" + Transform(C_Num) + "\\" +  Transform(D_Num) + " " + Transform(S_Num) + " " + Transform(H_Num) + " "  + Transform(XH) + ".data", ios::in); 
	for (int m = 1; m < H_Num + 1; m++)
	{
		for (int d = S_Num + 1; d < S_Num + D_Num + 1; d++)
		{
			infile >> Node[d].Supply[m];
		}
		for (int s = 1; s < S_Num + 1; s++)
		{
			infile >> Node[s].Demand[m]; 
		}
		for (int s = 1; s < S_Num + 1; s++)
		{
			for (int c = 1; c < C_Num + 1; c++)
			{
				infile >> Cust[s][c].Demand[m];
				Cust[s][c].Total_Demand += Cust[s][c].Demand[m];
			}
		}
	}
	for (int d = S_Num + 1; d < S_Num + D_Num + 1; d++)
	{
		infile >> Node[d].X;
	}
	for (int s = 1; s < S_Num + 1; s++)
	{
		infile >> Node[s].X;
	}
	for (int s = 1; s < S_Num + 1; s++)
	{
		Cust[s][0].X = Node[s].X;
		for (int c = 1; c < C_Num + 1; c++)
		{
			infile >> Cust[s][c].X;
		}
	}
	for (int d = S_Num + 1; d < S_Num + D_Num + 1; d++)
	{
		infile >> Node[d].Y;
	}
	for (int s = 1; s < S_Num + 1; s++)
	{
		infile >> Node[s].Y;
	}
	for (int s = 1; s < S_Num + 1; s++)
	{
		Cust[s][0].Y = Node[s].Y;
		for (int c = 1; c < C_Num + 1; c++)
		{
			infile >> Cust[s][c].Y;
		}
	}
	for (int s = 1; s < S_Num + 1; s++)
	{
		for (int c = 1; c < C_Num + 1; c++)
		{
			infile >> Cust[s][c].Earliest_ST;
		}
	}
	for (int s = 1; s < S_Num + 1; s++)
	{
		for (int c = 1; c < C_Num + 1; c++)
		{
			infile >> Cust[s][c].Latest_ST;
		}
	}
	infile.close();
}

void Distance_Matrix(int D_Num, int S_Num, int C_Num, int H_Num, First_Node*& Node, float**& First_Distance, float**& First_Nodes_Time, float**& First_Nodes_Cost,
	Second_Cust**& Cust, float***& Second_Distance, float***& Second_Cust_Time, float***& Second_Cust_Cost)
{//Calculate the distance, time and cost between nodes
	//the first-echelon nodes
	for (int i = 0; i < D_Num + S_Num + 1; i++)
	{
		for (int j = 0; j < D_Num + S_Num + 1; j++) 
		{
			First_Distance[i][j] = L;
			if (i != j && i != 0 && j != 0)
			{
				First_Distance[i][j] = sqrt((Node[i].X - Node[j].X) * (Node[i].X - Node[j].X) + (Node[i].Y - Node[j].Y) * (Node[i].Y - Node[j].Y));
			}
			First_Nodes_Time[i][j] = First_Distance[i][j] / BV_Speed;
			First_Nodes_Cost[i][j] = First_Distance[i][j] * BV_coe;
		}
	}
	//the second-echelon nodes
	for (int u = 1; u < S_Num + 1; u++)
	{
		for (int i = 0; i < C_Num + 1; i++) 
		{
			for (int j = 0; j < C_Num + 1; j++)
			{
				Second_Distance[u][i][j] = sqrt((Cust[u][i].X - Cust[u][j].X) * (Cust[u][i].X - Cust[u][j].X) + (Cust[u][i].Y - Cust[u][j].Y) * (Cust[u][i].Y - Cust[u][j].Y));
				Second_Cust_Time[u][i][j] = Second_Distance[u][i][j] / SV_Speed;
				Second_Cust_Cost[u][i][j] = Second_Distance[u][i][j] * SV_coe;
			}
		}
	}
}

void Update_First_Echelon_Time(int S_Num, int H_Num, int BV_Num, int SV_Num, First_Node* Node, First_Solution*& FR,  float**& First_Nodes_Time,Second_Solution& SR, float*** Second_Cust_Time)
{//update the first-echelon time parameters
	float** Latest_Time = new float*[S_Num + 1];
	for (int u = 1; u < S_Num + 1; u++)      
	{
		Latest_Time[u] = new float[H_Num + 1];
		for (int m = 1; m < H_Num + 1; m++)  
		{
			Latest_Time[u][m] = 18;         
		}
	}	
	for (int u = 1; u < S_Num + 1; u++)        
	{		
		for (int m = 1; m < H_Num + 1; m++)    
		{		
			for (int r = 1; r < SV_Num + 1; r++)
			{
				if (SR.node[u][r][1] > 0)
				{					
					SR.dt[u][r] = SR.at[u][r][SR.node[u][r][1]] - Second_Cust_Time[u][0][SR.node[u][r][1]];
					if (SR.dt[u][r] * SR.load[u][r][m] > 0 && Latest_Time[u][m] > SR.dt[u][r] * SR.load[u][r][m])
					{
						Latest_Time[u][m] = (SR.dt[u][r]-tope)* SR.load[u][r][m];
					}					
				}
			}
		}
	}
	float** Adjust_Time = new float*[BV_Num + 1];
	for (int k = 0; k < BV_Num + 1; k++)      
	{
		Adjust_Time[k] = new float[S_Num + 1];
		for (int u = 0; u < S_Num + 1; u++)   
		{
			Adjust_Time[k][u] = 18;
		}
	}
	for (int k = 1; k < BV_Num + 1; k++)
	{
		if (FR[k].satenum > 0)
		{
			FR[k].dt = 0;
			FR[k].at[FR[k].node[1]] = First_Nodes_Time[FR[k].dc][FR[k].node[1]];
			for (int m = 1; m < H_Num + 1; m++) {
				if (FR[k].supply[FR[k].node[1]][m] > 0)
					Node[FR[k].node[1]].at[m] = FR[k].at[FR[k].node[1]];
			}
			
			for (int i = 2; i < FR[k].satenum + 1; i++)
			{
				int s0 = FR[k].node[i - 1]; int s1 = FR[k].node[i];
				FR[k].at[s1] = FR[k].at[s0] + First_Nodes_Time[s0][s1];
				for (int m = 1; m < H_Num + 1; m++)
				{
					if (FR[k].supply[s1][m] > 0)Node[s1].at[m] = FR[k].at[s1];
				}
			}
			FR[k].time = FR[k].at[FR[k].node[FR[k].satenum]] + First_Nodes_Time[FR[k].node[FR[k].satenum]][FR[k].dc];
		}
	}
	for (int k = 1; k < BV_Num + 1; k++)       
	{
		for (int n = 1; n <= FR[k].satenum; n++)
		{
			int u = FR[k].node[n];			
			for (int m = 1; m < H_Num + 1; m++)
			{				
				if (FR[k].supply[u][m] == 1 && Adjust_Time[k][u] > Latest_Time[u][m] - Node[u].at[m])
				{
					Adjust_Time[k][u] = Latest_Time[u][m] - Node[u].at[m];
				}
			}
			if (Adjust_Time[k][0] > Adjust_Time[k][u])
			{
				Adjust_Time[k][0] = Adjust_Time[k][u];
			}			
		}
	}
	
	for (int k = 1; k < BV_Num + 1; k++)      
	{
		FR[k].dt += Adjust_Time[k][0];
		for (int n = 1; n <= FR[k].satenum; n++)
		{
			int u = FR[k].node[n];
			FR[k].at[u] += Adjust_Time[k][0];
			for (int m = 1; m < H_Num + 1; m++)
			{
				if (FR[k].supply[u][m] == 1)Node[u].at[m] =FR[k].at[u];
			}
		}
	}	
}

void Penalty_Cost(int S_Num, int H_Num, int SV_Num, Total_Cost& TC, First_Node* Node, First_Solution*& FR, Second_Solution& SR)
{//Calculate the total waiting cost
	TC.link_cost = 0;
	float** wait_cost = new float*[S_Num + 1];
	for (int u = 1; u < S_Num + 1; u++)     
	{
		wait_cost[u] = new float[H_Num + 1];
		for (int m = 1; m < H_Num + 1; m++)
		{
			wait_cost[u][m] = 0;
		}
	}
	
	for (int u = 1; u < S_Num + 1; u++)         
	{
		for (int m = 1; m < H_Num + 1; m++)     
		{
			for (int r = 1; r < SV_Num + 1; r++)
			{
				if (SR.node[u][r][1] > 0)
				{
					float temp = SR.load[u][r][m] * SR.dt[u][r] - Node[u].at[m] - tope;
					if (temp > 0 && wait_cost[u][m] < temp*Pel)
					{
						wait_cost[u][m] = temp * Pel;
					}
			    }				
			}
			TC.link_cost += wait_cost[u][m];
		}
	}
}

void Caculate_Total_Cost(int S_Num, int C_Num, int H_Num, int BV_Num, int SV_Num, Total_Cost &TC, First_Node* Node, First_Solution*& FR, Second_Cust**& Cust, Second_Solution& SR, float*** Second_Cust_Cost, float** First_Nodes_Cost)
{//calculate the total cost	
	for (int k = 1; k < BV_Num + 1; k++)
	{
		if (FR[k].satenum > 0)  
		{			
			TC.First_Vehicle_Cost += F1;			
			for (int u = 1; u <= FR[k].satenum + 1; u++)
			{
				TC.First_Route_Cost[k] += First_Nodes_Cost[FR[k].node[u - 1]][FR[k].node[u]];
			}
			TC.First_Total_Route_Cost += TC.First_Route_Cost[k];			
		}		
	}
	
	Penalty_Cost(S_Num, H_Num, SV_Num, TC, Node, FR, SR);	
	for (int u = 1; u < S_Num + 1; u++)     
	{
		for (int r = 1; r < SV_Num + 1; r++)
		{
			SR.node[u][r][0] = 0;        
			if (SR.node[u][r][1] != 0)  
			{				
				TC.Second_Vehicle_Cost[u] += F2;				
				for (int n = 1; n < C_Num + 1; n++)
				{					
					if (SR.node[u][r][n] > 0 || (SR.node[u][r][n] == 0 && SR.node[u][r][n - 1] > 0))
					{
						TC.Second_Route_Cost[u][r] += Second_Cust_Cost[u][SR.node[u][r][n]][SR.node[u][r][n - 1]];
					}
					else
					{
						break;
					}
				}				
			}
			TC.Second_ES_Route_Cost[u] += TC.Second_Route_Cost[u][r];
		}
		
		TC.Second_Total_Route_Cost += TC.Second_ES_Route_Cost[u];		
		TC.Second_Total_Vehicle_Cost += TC.Second_Vehicle_Cost[u];
	}	
	TC.TotalCost = TC.First_Total_Route_Cost + TC.First_Vehicle_Cost + TC.Second_Total_Route_Cost + TC.Second_Total_Vehicle_Cost + TC.link_cost;
	
}

int main()
{	
	long timeStart = GetTickCount64();
	First_Node* Node = new First_Node[D_Num + S_Num + 1];
	First_Solution* FR = new First_Solution[BV_Num + 1];
	int** cargo_orig = new int*[H_Num + 1];
	for (int m = 0; m < H_Num + 1; m++)
	{
		cargo_orig[m] = new int[D_Num + 1];
		for (int d = 0; d < D_Num + 1; d++)
		{
			cargo_orig[m][d] = 0;
		}
	}
	float** First_Distance = new float*[D_Num + S_Num + 1];
	float** First_Nodes_Time = new float*[D_Num + S_Num + 1];
	float** First_Nodes_Cost = new float*[D_Num + S_Num + 1];
	for (int i = 0; i < D_Num + S_Num + 1; i++)
	{
		First_Distance[i] = new float[D_Num + S_Num + 1];
		First_Nodes_Time[i] = new float[D_Num + S_Num + 1];
		First_Nodes_Cost[i] = new float[D_Num + S_Num + 1];
	}
	Second_Cust** Cust = new Second_Cust *[S_Num + 1];
	Second_Solution SR;
	float*** Second_Distance = new float**[S_Num + 1];
	float*** Second_Cust_Time = new float**[S_Num + 1];
	float*** Second_Cust_Cost = new float**[S_Num + 1];
	for (int u = 0; u < S_Num + 1; u++)
	{
		Cust[u] = new Second_Cust[C_Num + 1];
		Second_Distance[u] = new float*[C_Num + 1];
		Second_Cust_Time[u] = new float*[C_Num + 1];
		Second_Cust_Cost[u] = new float*[C_Num + 1];
		for (int i = 0; i < C_Num + 1; i++)
		{
			Second_Distance[u][i] = new float[C_Num + 1];
			Second_Cust_Time[u][i] = new float[C_Num + 1];
			Second_Cust_Cost[u][i] = new float[C_Num + 1];
		}
	}

	Total_Cost TC;
	Read_Data(D_Num, S_Num, C_Num, H_Num, XH, Node, Cust);
	Distance_Matrix(D_Num, S_Num, C_Num, H_Num, Node, First_Distance, First_Nodes_Time, First_Nodes_Cost, Cust, Second_Distance, Second_Cust_Time, Second_Cust_Cost);
	Sate_Arrive_Time_Limit(S_Num, C_Num, H_Num, Node, Cust, Second_Cust_Time);
	Cargo_Orig(D_Num, S_Num, H_Num, cargo_orig, Node);
	Initial_First__Route(D_Num, S_Num, H_Num, BV_Num, Node, FR, First_Distance, First_Nodes_Time, cargo_orig);
	Initial_Second_Route(S_Num, C_Num, H_Num, SV_Num, Node, FR, First_Nodes_Time, Cust, SR, Second_Cust_Time);

	Update_First_Echelon_Time(S_Num, H_Num, BV_Num, SV_Num, Node, FR, First_Nodes_Time, SR, Second_Cust_Time);
	Caculate_Total_Cost(S_Num, C_Num, H_Num, BV_Num, SV_Num, TC, Node, FR, Cust, SR, Second_Cust_Cost, First_Nodes_Cost);

	Total_Cost bestTC = TC;
	Total_Cost curTC = TC;
	float T0 = 100;
	int tIter = 5;

	double *removePro = new double[5];
	float *removeScore = new float[5];
	for (int i = 0; i < 5; i++)
	{
		removeScore[i] = 0;
		if (i == 0)removePro[i] = 0;
		else removePro[i] = 0.25;//初始值等分
	}

	int count = 0;

	Second_Solution bestSR;

	for (int u = 1; u < S_Num + 1; u++)
	{
		for (int i = 0; i < 5; i++)
		{
			removeScore[i] = 0;
		}

		int removeOperator;
		for (int a = 1; a <= sateIter; a++)
		{
			FirstEchelonDisturb(u, D_Num, S_Num, C_Num, H_Num, BV_Num, Node, FR, First_Nodes_Time, First_Nodes_Cost, removeOperator, removePro, removeScore, Cust, Second_Cust_Time);

			bool improve = 0;

			Second_Solution SR1;
			Initial_Second_Route(S_Num, C_Num, H_Num, SV_Num, Node, FR, First_Nodes_Time, Cust, SR1, Second_Cust_Time);
			for (int b = 1; b <= custIter; b++)
			{
				SecondEchelonDisturb(u, Node, Cust, SR1, Second_Cust_Time, Second_Cust_Cost);

				Total_Cost curTC1;
				Update_First_Echelon_Time(S_Num, H_Num, BV_Num, SV_Num, Node, FR, First_Nodes_Time, SR1, Second_Cust_Time);
				Caculate_Total_Cost(S_Num, C_Num, H_Num, BV_Num, SV_Num, curTC1, Node, FR, Cust, SR1, Second_Cust_Cost, First_Nodes_Cost);

				if (curTC1.TotalCost < bestTC.TotalCost)
				{
					bestTC = curTC1; improve = 1; bestSR = SR1;
					count = 0;
				}
				else
				{
					count += 1;
				}
				if (curTC1.TotalCost < curTC.TotalCost)
				{
					curTC = curTC1;
				}
				if (curTC1.TotalCost >= curTC.TotalCost && (rand() % 99 + 1) / 100 < exp((curTC1.TotalCost - curTC.TotalCost) / T0))
				{
					curTC = curTC1;
				}
				tIter--;
				if (tIter == 0)
				{
					T0 = T0 * cooling;
					tIter = 5;
				}
				if (count == continuIter)break;
			}
			if (count == continuIter)break;
			if (improve == 1)
			{
				removeScore[removeOperator] += 100;
			}

		}

		long totalNum = 0;
		for (int i = 1; i < 5; i++)totalNum += removeScore[i];
		for (int i = 1; i < 5; i++)
		{
			removePro[i] = alpha * removePro[i] + (1 - alpha)*(removeScore[i] / totalNum);
		}

		if (count == continuIter)break;
	}

	long timeFinish = GetTickCount64();

	ofstream outfile;
	outfile.open("result.csv", ios::app);
	outfile << "\n";
	outfile << D_Num << ',' << S_Num << ',' << H_Num << ',' << C_Num << ',' << XH << ',' << (timeFinish - timeStart) / 1000.00 << ','
		<< bestTC.TotalCost << ',' << bestTC.First_Total_Route_Cost + bestTC.First_Vehicle_Cost << ',' << bestTC.link_cost << ',' << bestTC.Second_Total_Route_Cost + bestTC.Second_Total_Vehicle_Cost;
	outfile.flush();
	outfile.close();	
}