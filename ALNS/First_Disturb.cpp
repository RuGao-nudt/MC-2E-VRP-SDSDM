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

//***************************************** The first-echelon perturbation operators ************************************

void RouletteWheelSelection(int &removeOperator, double *&removePro, float *&removeScore)
{//The roulette algorithm	
	double r;	
	LARGE_INTEGER seed;              
	QueryPerformanceFrequency(&seed);
	QueryPerformanceCounter(&seed);  
	srand(seed.QuadPart);
	r = rand() % (100) / (float)(100);
	double pTotal = 0.0;
	
	for (int a = 1; a < 5; a++)
	{
		pTotal += removePro[a];
		if (pTotal > r)
		{
			removeOperator = a;
			removeScore[a] += 1;
			break;
		}
	}	
}

void Sate_Supp_Dema(int u, int H_Num, int BV_Num, First_Solution*& FR, Sate_Supply_Demand *&supplyDemand)
{//Obtain the supply and demand relationship of a certain satellite
	for (int r = 1; r <= BV_Num; r++)
	{
		if (FR[r].node[0] > 0)
		{
			for (int i = 1; i < S_Num + 1; i++)
			{
				if (FR[r].node[i] == u)
				{
					supplyDemand[u].car[0] += 1;
					supplyDemand[u].car[supplyDemand[u].car[0]] = r;
					supplyDemand[u].supply[r] = FR[r].dc;
					supplyDemand[u].index[r] = i;
					for (int m = 1; m <= H_Num; m++)
					{
						if (FR[r].supply[u][m] == 1)
						{
							supplyDemand[u].cargoTpye[r][0] += 1;
							supplyDemand[u].cargoTpye[r][supplyDemand[u].cargoTpye[r][0]] = m;
						}
					}
					supplyDemand[u].num += 1;
					break;
				}
			}
		}
	}
}

void After_Remove(int u, int r, int S_Num, First_Node* Node, First_Solution*& FR, int**& removalDemand, float **& dcSupply, Sate_Supply_Demand *&supplyDemand)
{//Record the removal demands and supply-demand, and update the route nodes
	int dc = supplyDemand[u].supply[r];
	int index = supplyDemand[u].index[r];
	
	for (int h = 1; h <= supplyDemand[u].cargoTpye[r][0]; h++)
	{
		int m = supplyDemand[u].cargoTpye[r][h];
		removalDemand[u][0] += 1;
		removalDemand[u][m] = Node[u].Demand[m];
		dcSupply[dc - S_Num][m] += Node[u].Demand[m];
		FR[r].restC += Node[u].Demand[m];
		FR[r].supply[u][m] = 0;
	}

	for (int n = index; n < S_Num + 1; n++)
	{
		FR[r].node[n] = FR[r].node[n + 1];
	}
	FR[r].satenum -= 1;
	
	for (int i = 1; i < FR[r].satenum + 1; i++)
	{
		int sate = FR[r].node[i];
		supplyDemand[sate].index[r] = i;
	}
}

void Random_Removal_Operator(int u, int D_Num, int S_Num, int C_Num, int H_Num, int BV_Num, First_Node* Node, First_Solution*& FR, int**& removalDemand, float **& dcSupply, Sate_Supply_Demand *&supplyDemand)
{//random removal operator
	if (supplyDemand[u].num > 0)
	{
		int removNum = rand() % supplyDemand[u].num + 1;
		for (int i = 1; i <= removNum; i++)
		{
			int r = supplyDemand[u].car[i];
			After_Remove(u, r, S_Num, Node, FR, removalDemand, dcSupply, supplyDemand);
		}
	}
}

void Worst_Removal_Operator(int u, int D_Num, int S_Num, int C_Num, int H_Num, int BV_Num, First_Node* Node, First_Solution*& FR, int**& removalDemand, float **& dcSupply, Sate_Supply_Demand *&supplyDemand, float** First_Nodes_Cost)
{//worst removal operator
	if (supplyDemand[u].num > 0)
	{
		int totalNum = supplyDemand[u].num;
		float * costChange = new float[totalNum + 1];
		int * costSort = new int[totalNum + 1];
		for (int i = 0; i < totalNum + 1; i++)
		{
			costChange[i] = 0;
			costSort[i] = 0;
		}		
		for (int j = 1; j < totalNum + 1; j++)
		{
			int r = supplyDemand[u].car[j];
			int index = supplyDemand[u].index[r];
			int pre_node = FR[r].node[index - 1];
			int lat_node = FR[r].node[index + 1];
			costSort[0] += 1;
			costSort[costSort[0]] = r;
			costChange[costSort[0]] = First_Nodes_Cost[pre_node][u] + First_Nodes_Cost[u][lat_node] - First_Nodes_Cost[pre_node][lat_node];
		}
		
		for (int a = 1; a < totalNum; a++)
		{
			for (int b = a + 1; b < totalNum + 1; b++)
			{
				if (costChange[a] < costChange[b])
				{
					float tempChange = costChange[b];
					costChange[b] = costChange[a];
					costChange[a] = tempChange;

					int tempSort = costSort[b];
					costSort[b] = costSort[a];
					costSort[a] = tempSort;
				}
			}
		}
		
		int removNum = rand() % supplyDemand[u].num + 1;
		for (int i = 1; i < removNum + 1; i++)
		{
			int r = costSort[i];
			After_Remove(u, r, S_Num, Node, FR, removalDemand, dcSupply, supplyDemand);
		}
	}

}

void Relevance_Removal_Operator(int D_Num, int S_Num, int C_Num, int H_Num, int BV_Num, First_Node* Node, First_Solution*& FR, int**& removalDemand, float **& dcSupply, Sate_Supply_Demand *&supplyDemand)
{//relevance removal operator	
	int guidCargo = rand() % H_Num + 1;	
	int sateNum = rand() % S_Num + 1;
	for (int u = 1; u <= sateNum; u++)
	{
		int h = 0;
		for (int i = 1; i <= supplyDemand[u].car[0]; i++)
		{
			int r = supplyDemand[u].car[i];
			for (int m1 = 1; m1 <= supplyDemand[u].cargoTpye[r][0]; m1++)
			{
				if (supplyDemand[u].cargoTpye[r][m1] == guidCargo)
				{
					After_Remove(u, r, S_Num, Node, FR, removalDemand, dcSupply, supplyDemand);
					h = 1;
					break;
				}
			}
			if (h == 1)break;
		}
	}
}

void Oriented_Removal_Operator(int u, int S_Num, int BV_Num, First_Node* Node, First_Solution*& FR, int**& removalDemand, float **& dcSupply, Sate_Supply_Demand *&supplyDemand)
{//With the Same Satellite and CDC Oriented Removal
	if (supplyDemand[u].num > 0)
	{
		int guidDC; int guidCar;		
		int removNum1 = rand() % supplyDemand[u].num + 1;
		for (int i = 1; i <= removNum1; i++)
		{
			int r = supplyDemand[u].car[i];
			After_Remove(u, r, S_Num, Node, FR, removalDemand, dcSupply, supplyDemand);
			if (i == removNum1)
			{
				guidDC = supplyDemand[u].supply[r];
				guidCar = r;
			}
		}
		
		if (FR[guidCar].satenum > 0)
		{
			int removNum2 = rand() % FR[guidCar].satenum + 1;
			for (int i = 1; i <= removNum2; i++)
			{
				int sate = FR[guidCar].node[i];
				if (sate > 0 && sate < S_Num + 1)
				{
					After_Remove(sate, guidCar, S_Num, Node, FR, removalDemand, dcSupply, supplyDemand);
				}
			}
		}
	}
}

void Select_DC(int u, int m1, int& dc, int**& removalDemand, float **& dcSupply)
{//the CDC is selected for new route	
	int *count = new int[D_Num + 1];
	for (int d = 1; d < D_Num + 1; d++)
	{
		count[d] = 0;
	}
	for (int d = 1; d < D_Num + 1; d++)
	{
		if (dcSupply[d][m1] >= removalDemand[u][m1])
		{
			for (int m = 1; m < H_Num + 1; m++)
			{
				if (dcSupply[d][m] >= removalDemand[u][m] && removalDemand[u][m] > 0)
				{
					count[d] += 1;
				}
			}
		}
	}
	dc = 1;
	for (int d = 1; d < D_Num + 1; d++)
	{
		if (count[dc] < count[d])
		{
			dc = d;
		}
	}
	delete[]count;
}

void Search_Lacation(int u, int m, First_Node* Node, First_Solution*& FR, int**& removalDemand, float **& dcSupply, InsertNode &finalInsert, float** First_Nodes_Time, float** First_Nodes_Cost)
{//Search for the node with the least cost for change
	InsertNode *routeInsert = new InsertNode[BV_Num + 1];
	for (int r = 1; r < BV_Num + 1; r++)
	{
		float addCost = 10000.0;
		int index = 0;
		if (FR[r].dc > S_Num)
		{
			int dc = FR[r].dc;			
			if (removalDemand[u][m] > 0 && dcSupply[dc - S_Num][m] >= removalDemand[u][m] && FR[r].restC - removalDemand[u][m] > 0)
			{
				int * feas = new int[FR[r].satenum + 2];				
				for (int i = 1; i <= FR[r].satenum + 1; i++)
				{
					int sate = FR[r].node[i];
					feas[i] = 0;
					if (sate == u)
					{
						feas[i] = 1;
						addCost = 0;
						index = i;
						break;
					}
					else
					{
						if (i == 1)
						{							
							if (FR[r].time + First_Nodes_Time[dc][u] + First_Nodes_Time[u][sate] - First_Nodes_Time[dc][sate] <= T1 && First_Nodes_Time[dc][u] <= Node[u].at_limit[m])
							{
								for (int j = 1; j < FR[r].satenum + 1; j++)
								{
									feas[i] = 1;
									int sate1 = FR[r].node[j];
									float at_time = FR[r].at[sate1] + First_Nodes_Time[dc][u] + First_Nodes_Time[u][sate] - First_Nodes_Time[dc][sate];
									if (at_time > Node[sate1].at_limit[m])
									{
										feas[i] = 0;										
										break;
									}
								}
							}
							if (feas[i] == 1)
							{
								if (addCost > First_Nodes_Cost[dc][u] + First_Nodes_Cost[u][sate] - First_Nodes_Cost[dc][sate])
								{
									addCost = First_Nodes_Cost[dc][u] + First_Nodes_Cost[u][sate] - First_Nodes_Cost[dc][sate];
									index = i;									
								}
							}
						}
						
						if (i == FR[r].satenum + 1)
						{
							int sate1 = FR[r].node[i - 1];
							if (FR[r].time + First_Nodes_Time[sate1][u] + First_Nodes_Time[u][dc] - First_Nodes_Time[sate1][dc] <= T1 && FR[r].at[sate1] + First_Nodes_Time[sate1][u] <= Node[u].at_limit[m])
							{
								feas[i] = 1;
							}
							if (feas[i] == 1)
							{
								if (addCost > First_Nodes_Cost[sate1][u] + First_Nodes_Cost[u][dc] - First_Nodes_Cost[sate1][dc])
								{
									addCost = First_Nodes_Cost[sate1][u] + First_Nodes_Cost[u][dc] - First_Nodes_Cost[sate1][dc];
									index = i;									
								}
							}
						}
						
						if (i > 1 && i < FR[r].satenum + 1)
						{
							int pre_sate = FR[r].node[i - 1];
							if (FR[r].time + First_Nodes_Time[pre_sate][u] + First_Nodes_Time[u][sate] - First_Nodes_Time[pre_sate][sate] <= T1 && FR[r].at[pre_sate] + First_Nodes_Time[pre_sate][u] <= Node[u].at_limit[m])
							{
								for (int j = i; j < FR[r].satenum + 1; j++)
								{
									feas[i] = 1;
									int sate1 = FR[r].node[j];
									float at_time = FR[r].at[sate1] + First_Nodes_Time[pre_sate][u] + First_Nodes_Time[u][sate] - First_Nodes_Time[pre_sate][sate];
									if (at_time > Node[sate1].at_limit[m])
									{
										feas[i] = 0;										
										break;
									}
								}
							}
							if (feas[i] == 1)
							{
								if (addCost > First_Nodes_Cost[pre_sate][u] + First_Nodes_Cost[u][sate] - First_Nodes_Cost[pre_sate][sate])
								{
									addCost = First_Nodes_Cost[pre_sate][u] + First_Nodes_Cost[u][sate] - First_Nodes_Cost[pre_sate][sate];
									index = i;									
								}
							}
						}
					}

				}
				delete feas;
			}
		}
		else
		{
			int dc;
			Select_DC(u, m, dc, removalDemand, dcSupply);
			dc = dc + S_Num;
			
			if (addCost > 2 * First_Nodes_Cost[dc][u])
			{
				index = 1;
				addCost = 2 * First_Nodes_Cost[dc][u];
				
			}
		}
		
		if (index != 0)
		{
			routeInsert[r].car = r;
			routeInsert[r].pos = index;
			routeInsert[r].addtime = addCost;
		}
	}
	
	int finalRoute = 1;
	for (int r = 1; r < BV_Num + 1; r++)
	{
		if (routeInsert[finalRoute].addtime > routeInsert[r].addtime)
		{
			finalRoute = r;
		}
	}
	
	finalInsert.car = finalRoute;
	finalInsert.addtime = routeInsert[finalRoute].addtime;
	finalInsert.pos = routeInsert[finalRoute].pos;

}

void Repair_Operator(First_Node* Node, First_Solution*& FR, int**& removalDemand, float **& dcSupply, Sate_Supply_Demand *&supplyDemand, float** First_Nodes_Time, float** First_Nodes_Cost)
{//greedy repair operator
	for (int u = 1; u < S_Num + 1; u++)
	{
		for (int m = 1; m < H_Num + 1; m++)
		{
			if (removalDemand[u][m] > 0)
			{				
				InsertNode finalInsert;				
				Search_Lacation(u, m, Node, FR, removalDemand, dcSupply, finalInsert, First_Nodes_Time, First_Nodes_Cost);				
				int r = finalInsert.car;
				int index = finalInsert.pos;				
				if (index > 0)
				{
					if (FR[r].dc > 0)
					{
						int dc = FR[r].dc;						
						if (dcSupply[dc - S_Num][m] >= removalDemand[u][m])
						{
							dcSupply[dc - S_Num][m] -= removalDemand[u][m];
							FR[r].restC -= removalDemand[u][m];
							FR[r].supply[u][m] = 1;
							removalDemand[u][m] = 0;
							if (finalInsert.addtime > 0)
							{
								FR[r].satenum += 1;
								int pre_node = FR[r].node[index - 1];
								if (index == 1)
								{
									FR[r].at[u] = First_Nodes_Time[pre_node][u];
								}
								else
								{
									FR[r].at[u] = FR[r].at[pre_node] + First_Nodes_Time[pre_node][u];
								}
								for (int i = FR[r].satenum + 1; i > index; i--)
								{
									FR[r].node[i] = FR[r].node[i - 1];
								}
								FR[r].node[index] = u;
								for (int i = index + 1; i < FR[r].satenum + 1; i++)
								{
									int pre_sate = FR[r].node[i - 1];
									int sate = FR[r].node[i];
									FR[r].at[sate] = FR[r].at[pre_sate] + First_Nodes_Time[pre_sate][sate];
								}
								FR[r].time = FR[r].at[FR[r].node[FR[r].satenum]] + First_Nodes_Time[FR[r].node[FR[r].satenum]][dc];
							}
							
							for (int m1 = 1; m1 < H_Num + 1; m1++)
							{
								if (m1 != m && removalDemand[u][m1] <= dcSupply[dc - S_Num][m1] && FR[r].restC - removalDemand[u][m1] > 0 && removalDemand[u][m1] > 0)
								{
									FR[r].supply[u][m1] = 1;									
									FR[r].restC -= removalDemand[u][m1];
									dcSupply[dc - S_Num][m1] -= removalDemand[u][m1];
									removalDemand[u][m1] = 0;
								}
							}
						}
					}
					else
					{
						int dc;
						Select_DC(u, m, dc, removalDemand, dcSupply);
						dc = dc + S_Num;	
						
						if (dcSupply[dc - S_Num][m] >= removalDemand[u][m])
						{
							FR[r].dc = dc;
							dcSupply[dc - S_Num][m] -= removalDemand[u][m];
							FR[r].restC -= removalDemand[u][m];
							removalDemand[u][m] = 0;
							FR[r].supply[u][m] = 1;
							FR[r].satenum += 1;
							FR[r].node[1] = u;
							FR[r].node[0] = dc;
							FR[r].node[2] = dc;
							FR[r].time += First_Nodes_Time[dc][u];
							FR[r].at[u] = First_Nodes_Time[dc][u];
							for (int m1 = 1; m1 < H_Num + 1; m1++)
							{
								if (m1 != m && removalDemand[u][m1] <= dcSupply[dc - S_Num][m1] && FR[r].restC - removalDemand[u][m1] > 0 && removalDemand[u][m1] != 0)
								{
									FR[r].supply[u][m1] = 1;
									FR[r].restC -= removalDemand[u][m1];
									dcSupply[dc - S_Num][m1] -= removalDemand[u][m1];
									removalDemand[u][m1] = 0;
								}
							}
						}
					}
				}				
			}
		}
	}

}

void Feasi_Lacation(int u, int m, First_Node* Node, First_Solution*& FR, int**& removalDemand, float **& dcSupply, InsertNode &finalInsert, float** First_Nodes_Time, float** First_Nodes_Cost) {
//Search for feasible nodes
	vector<vector<int>>all_pos(BV_Num + 1);

	for (int r = 1; r < BV_Num + 1; r++) {

		if (FR[r].dc > S_Num)
		{
			int dc = FR[r].dc;

			if (removalDemand[u][m] > 0 && dcSupply[dc - S_Num][m] >= removalDemand[u][m] && FR[r].restC - removalDemand[u][m] > 0)   
			{
				int * feas = new int[FR[r].satenum + 2];
				
				for (int i = 1; i <= FR[r].satenum + 1; i++)
				{
					int sate = FR[r].node[i];
					feas[i] = 0;
					if (sate == u)
					{
						feas[i] = 1;						
						all_pos[r].clear();
						all_pos[r].push_back(1000 + i);
						break;
					}
					else
					{						
						if (i == 1)
						{						
							if (FR[r].time + First_Nodes_Time[dc][u] + First_Nodes_Time[u][sate] - First_Nodes_Time[dc][sate] <= T1 && First_Nodes_Time[dc][u] <= Node[u].at_limit[m])
							{
								for (int j = 1; j < FR[r].satenum + 1; j++)
								{
									feas[i] = 1;
									int sate1 = FR[r].node[j];
									float at_time = FR[r].at[sate1] + First_Nodes_Time[dc][u] + First_Nodes_Time[u][sate] - First_Nodes_Time[dc][sate];
									if (at_time > Node[sate1].at_limit[m])
									{
										feas[i] = 0;
										
										break;
									}
								}
							}
							if (feas[i] == 1)
							{
								all_pos[r].push_back(i);
							}
						}
						
						if (i == FR[r].satenum + 1)
						{
							int sate1 = FR[r].node[i - 1];
							if (FR[r].time + First_Nodes_Time[sate1][u] + First_Nodes_Time[u][dc] - First_Nodes_Time[sate1][dc] <= T1 && FR[r].at[sate1] + First_Nodes_Time[sate1][u] <= Node[u].at_limit[m])
							{
								feas[i] = 1;
							}
							if (feas[i] == 1)
							{
								all_pos[r].push_back(i);
							}
						}
						
						if (i > 1 && i < FR[r].satenum + 1)
						{
							int pre_sate = FR[r].node[i - 1];
							if (FR[r].time + First_Nodes_Time[pre_sate][u] + First_Nodes_Time[u][sate] - First_Nodes_Time[pre_sate][sate] <= T1 && FR[r].at[pre_sate] + First_Nodes_Time[pre_sate][u] <= Node[u].at_limit[m])
							{
								for (int j = i; j < FR[r].satenum + 1; j++)
								{
									feas[i] = 1;
									int sate1 = FR[r].node[j];
									float at_time = FR[r].at[sate1] + First_Nodes_Time[pre_sate][u] + First_Nodes_Time[u][sate] - First_Nodes_Time[pre_sate][sate];
									if (at_time > Node[sate1].at_limit[m])
									{
										feas[i] = 0;										
										break;
									}
								}
							}
							if (feas[i] == 1)
							{
								all_pos[r].push_back(i);
							}
						}
					}

				}
				delete feas;
			}
		}
		else {
			all_pos[r].push_back(1);
			for (int r0 = 1; r0 < r; r0++) {
				if ((all_pos[r0].size() > 0) && (FR[r].dc < S_Num)) {
					all_pos[r].pop_back();
					break;
				}
			}

		}
	}

	int num = 0;
	for (int n = 1; n < BV_Num + 1; n++) {
		num += all_pos[n].size();
	}
	int n0 = rand() % num + 1;	
	int count = 0;
	for (int n = 1; n < BV_Num + 1; n++)
	{
		count += all_pos[n].size();
		if (count >= n0) {
			finalInsert.car = n;
			finalInsert.pos = all_pos[n][all_pos[n].size() - (count - n0) - 1];
			break;
		}
	}
	
	all_pos.clear();

}

void Random_Repair_Operator(First_Node* Node, First_Solution*& FR, int**& removalDemand, float **& dcSupply, Sate_Supply_Demand *&supplyDemand, float** First_Nodes_Time, float** First_Nodes_Cost)
{//random repair operator	
	for (int u = 1; u < S_Num + 1; u++) {
		
		for (int m = 1; m < H_Num + 1; m++) {
			if (removalDemand[u][m] > 0) {
				InsertNode finalInsert;				
				Feasi_Lacation(u, m, Node, FR, removalDemand, dcSupply, finalInsert, First_Nodes_Time, First_Nodes_Cost);				
				int r = finalInsert.car;
				int index = finalInsert.pos;				
				if (index > 0)
				{
					if (FR[r].dc > 0)
					{
						int dc = FR[r].dc;						
						if (dcSupply[dc - S_Num][m] >= removalDemand[u][m])
						{
							if (index > 1000) {
								index -= 1000;
								dcSupply[dc - S_Num][m] -= removalDemand[u][m];
								FR[r].restC -= removalDemand[u][m];
								FR[r].supply[u][m] = 1;
								removalDemand[u][m] = 0;
							}
							else {
								dcSupply[dc - S_Num][m] -= removalDemand[u][m];
								FR[r].restC -= removalDemand[u][m];
								FR[r].supply[u][m] = 1;
								removalDemand[u][m] = 0;
								if (finalInsert.addtime > 0)
								{
									FR[r].satenum += 1;
									int pre_node = FR[r].node[index - 1];
									if (index == 1)
									{
										FR[r].at[u] = First_Nodes_Time[pre_node][u];
									}
									else
									{
										FR[r].at[u] = FR[r].at[pre_node] + First_Nodes_Time[pre_node][u];
									}
									for (int i = FR[r].satenum + 1; i > index; i--)
									{
										FR[r].node[i] = FR[r].node[i - 1];
									}
									FR[r].node[index] = u;
									for (int i = index + 1; i < FR[r].satenum + 1; i++)
									{
										int pre_sate = FR[r].node[i - 1];
										int sate = FR[r].node[i];
										FR[r].at[sate] = FR[r].at[pre_sate] + First_Nodes_Time[pre_sate][sate];
									}
									FR[r].time = FR[r].at[FR[r].node[FR[r].satenum]] + First_Nodes_Time[FR[r].node[FR[r].satenum]][dc];
								}
								
								for (int m1 = 1; m1 < H_Num + 1; m1++)
								{
									if (m1 != m && removalDemand[u][m1] <= dcSupply[dc - S_Num][m1] && FR[r].restC - removalDemand[u][m1] > 0 && removalDemand[u][m1] > 0)
									{
										FR[r].supply[u][m1] = 1;										
										FR[r].restC -= removalDemand[u][m1];
										dcSupply[dc - S_Num][m1] -= removalDemand[u][m1];
										removalDemand[u][m1] = 0;
									}
								}
							}
						}
					}
					else
					{
						int dc;
						Select_DC(u, m, dc, removalDemand, dcSupply);
						dc = dc + S_Num;							
						if (dcSupply[dc - S_Num][m] >= removalDemand[u][m])
						{
							FR[r].dc = dc;
							dcSupply[dc - S_Num][m] -= removalDemand[u][m];
							FR[r].restC -= removalDemand[u][m];
							removalDemand[u][m] = 0;
							FR[r].supply[u][m] = 1;
							FR[r].satenum += 1;
							FR[r].node[1] = u;
							FR[r].node[0] = dc;
							FR[r].node[2] = dc;
							FR[r].time += First_Nodes_Time[dc][u];
							FR[r].at[u] = First_Nodes_Time[dc][u];
							for (int m1 = 1; m1 < H_Num + 1; m1++)
							{
								if (m1 != m && removalDemand[u][m1] <= dcSupply[dc - S_Num][m1] && FR[r].restC - removalDemand[u][m1] > 0 && removalDemand[u][m1] != 0)
								{
									FR[r].supply[u][m1] = 1;
									FR[r].restC -= removalDemand[u][m1];
									dcSupply[dc - S_Num][m1] -= removalDemand[u][m1];
									removalDemand[u][m1] = 0;
								}
							}
						}
					}
				}
			}
		}
	}
}

void Adjust_Operator(int r, First_Node* Node, First_Solution*& FR, float** First_Nodes_Time, Second_Cust**& Cust, float*** Second_Cust_Time)
{//time adjustment operator
	int sateNum = FR[r].satenum;
	
	float * earlyTime = new float[S_Num + 1];
	for (int s = 0; s < S_Num + 1; s++)
	{
		earlyTime[s] = 18;
	}
	for (int s = 1; s < sateNum + 1; s++)
	{
		int u = FR[r].node[s];
		int cust = 0;
		int * goods = new int[H_Num + 1];
		for (int m = 1; m < H_Num + 1; m++)
		{
			if (FR[r].supply[u][m] == 1)
			{
				goods[m] = m;
			}
		}
		for (int c = 1; c < C_Num + 1; c++)
		{
			bool ok = 0;
			for (int m = 1; m < H_Num + 1; m++)
			{
				if (Cust[u][c].Demand[m] > 0 && goods[m] == m)
				{
					ok = 1;
					continue;
				}
			}
			if (earlyTime[u] >= Cust[u][c].Earliest_ST && ok)
			{
				earlyTime[u] = Cust[u][c].Earliest_ST;
				cust = c;
			}
		}
		earlyTime[u] -= Second_Cust_Time[u][0][cust];
		delete[]goods;
	}
	
	for (int s1 = 1; s1 < sateNum; s1++)
	{
		int u1 = FR[r].node[s1];
		float u1_Time = earlyTime[u1];
		int exchange_node = 0;
		for (int s2 = s1 + 1; s2 < sateNum + 1; s2++)
		{
			int u2 = FR[r].node[s2];
			if (u1_Time > earlyTime[u2])
			{
				u1_Time = earlyTime[u2];
				exchange_node = s2;
			}
		}
		
		if (exchange_node != 0)
		{
			FR[r].node[s1] = FR[r].node[exchange_node];
			FR[r].node[exchange_node] = u1;
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
	delete[]earlyTime;
}

void FirstEchelonDisturb(int u, int D_Num, int S_Num, int C_Num, int H_Num, int BV_Num, First_Node* Node, First_Solution*& FR, float** First_Nodes_Time, float** First_Nodes_Cost, int &removeOperator, double *&removePro, float *&removeScore, Second_Cust**& Cust, float*** Second_Cust_Time)
{//The total function of the first-echelon disturbance
	int ** removalDemand = new int*[S_Num + 1];
	float ** dcSupply = new float*[D_Num + 1];
	for (int s = 0; s < D_Num + 1; s++)
	{
		dcSupply[s] = new float[H_Num + 1];
		for (int m = 0; m < H_Num + 1; m++)
		{
			dcSupply[s][m] = 0;
		}
	}
	for (int s = 0; s < S_Num + 1; s++)
	{
		removalDemand[s] = new int[H_Num + 1];
		for (int m = 0; m < H_Num + 1; m++)
		{
			removalDemand[s][m] = 0;
		}
	}
	for (int s = 1; s < D_Num + 1; s++)
	{
		for (int m = 1; m < H_Num + 1; m++)
		{
			dcSupply[s][m] = Node[s + S_Num].Supply[m];
		}
	}
	
	for (int r = 1; r <= BV_Num; r++)
	{
		if (FR[r].node[0] > 0)
		{
			int dc = FR[r].dc;
			for (int i = 1; i < S_Num + 1; i++)
			{
				int sate = FR[r].node[i];
				if (sate > 0 && sate < S_Num + 1)
				{
					for (int m = 1; m < H_Num + 1; m++)
					{
						if (FR[r].supply[sate][m] == 1)
						{
							dcSupply[dc - S_Num][m] -= Node[sate].Demand[m];
						}
					}
				}
			}
		}
	}
	Sate_Supply_Demand *supplyDemand = new Sate_Supply_Demand[S_Num + 1];
	
	for (int sate = 1; sate < S_Num + 1; sate++)
	{
		Sate_Supp_Dema(sate, H_Num, BV_Num, FR, supplyDemand);
	}
	
	removeOperator = 0;
	RouletteWheelSelection(removeOperator, removePro, removeScore);
	
	switch (removeOperator)
	{
	case 1:Random_Removal_Operator(u, D_Num, S_Num, C_Num, H_Num, BV_Num, Node, FR, removalDemand, dcSupply, supplyDemand); break;
	case 2:Worst_Removal_Operator(u, D_Num, S_Num, C_Num, H_Num, BV_Num, Node, FR, removalDemand, dcSupply, supplyDemand, First_Nodes_Cost); break;
	case 3:Relevance_Removal_Operator(D_Num, S_Num, C_Num, H_Num, BV_Num, Node, FR, removalDemand, dcSupply, supplyDemand); break;
	case 4:Oriented_Removal_Operator(u, S_Num, BV_Num, Node, FR, removalDemand, dcSupply, supplyDemand); break;
	}
	
	
	for (int r = 1; r < BV_Num + 1; r++)
	{
		if (FR[r].satenum > 0)
		{
			FR[r].time = 0;
			for (int i = 0; i < FR[r].satenum + 1; i++)
			{
				if (i > 0)
				{
					FR[r].at[FR[r].node[i]] = FR[r].time;
				}
				FR[r].time += First_Nodes_Time[FR[r].node[i]][FR[r].node[i + 1]];
			}
		}
		else
		{
			if (FR[r].dc > 0)
			{
				
				FR[r].node[0] = 0;
				FR[r].dc = 0;
				FR[r].time = 0;
				FR[r].dt = 0;
				FR[r].restC = Q1;
			}
		}
	}
	
	Repair_Operator(Node, FR, removalDemand, dcSupply, supplyDemand, First_Nodes_Time, First_Nodes_Cost);
		
	for (int r = 1; r < BV_Num + 1; r++)
	{
		if (FR[r].satenum > 0)
		{
			Adjust_Operator(r, Node, FR, First_Nodes_Time, Cust, Second_Cust_Time);
		}
	}
	Calculate_Cargo_Arrive_Time(S_Num, H_Num, BV_Num, Node, FR);//Ö÷ÒªÊÇNode[u0].at[m0] = FR[r0].at[u0]
	
	delete[]removalDemand;
	delete[]dcSupply;
}