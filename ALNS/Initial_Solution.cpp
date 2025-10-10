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

void Sate_Arrive_Time_Limit(int S_Num, int C_Num, int H_Num, First_Node*& Node, Second_Cust**& Cust, float*** Second_Cust_Time)
{//The arrival time limit of satellites
	///卡车到达中转仓库的最晚时间限制；
	for (int u0 = 1; u0 < S_Num + 1; u0++)
	{
		for (int i = 1; i < C_Num + 1; i++)
		{
			float temp = Cust[u0][i].Latest_ST - Second_Cust_Time[u0][0][i] - tope;
			for (int m0 = 1; m0 < H_Num + 1; m0++)
			{
				if (Cust[u0][i].Demand[m0] > 0 && Node[u0].at_limit[m0] > temp)
				{
					Node[u0].at_limit[m0] = temp;
				}
			}
		}
	}
}

//########################################## the first-echelon initial solution ##########################################

void Caculate_Nodes_Need(int D_Num, int S_Num, int H_Num, float**& Sate_Demand, int**& First_Demand_Node, int& unsatisfied_sate)
{//Calculate the multi-commodity demand for satellites	
	for (int m1 = 0; m1 < H_Num + 1; m1++)         
	{
		for (int u1 = 0; u1 < S_Num + 1; u1++)     
		{
			First_Demand_Node[m1][u1] = 0;       
		}
	}

	for (int m0 = 1; m0 < H_Num + 1; m0++)
	{
		for (int u0 = 1; u0 < S_Num + 1; u0++)
		{
			if (Sate_Demand[u0][m0] > 0)
			{
				First_Demand_Node[m0][0] += 1;
				First_Demand_Node[m0][First_Demand_Node[m0][0]] = u0;
			}
		}
		unsatisfied_sate += First_Demand_Node[m0][0];

	}

}

void Cargo_Orig(int D_Num, int S_Num, int H_Num, int**& cargo_orig, First_Node*& Node)
{//Determine the source of supply for the commodities
	for (int m0 = 1; m0 < H_Num + 1; m0++)
	{
		for (int d0 = S_Num + 1; d0 < S_Num + D_Num + 1; d0++)
		{
			if (Node[d0].Supply[m0] > 0)
			{
				cargo_orig[m0][0] += 1;  
				cargo_orig[m0][cargo_orig[m0][0]] = d0;
			}
		}
	}
}

void Caculate_E(int D_Num, int S_Num, int H_Num, float** First_Distance, float** Sate_Demand, float** DC_Supply, float**& tran_effi, int& maxeffi, int& dc_index, int& sate_index)
{//Calculate transportation efficiency
	for (int d0 = 1; d0 < D_Num + 1; d0++)
	{
		for (int s0 = 1; s0 < S_Num + 1; s0++)
		{
			float mm = 0;
			for (int m0 = 1; m0 < H_Num + 1; m0++)
			{
				if (DC_Supply[S_Num + d0][m0] >= Sate_Demand[s0][m0])
				{
					mm += Sate_Demand[s0][m0];
				}
			}
			tran_effi[d0][s0] = mm / First_Distance[d0][s0];
			if (tran_effi[d0][s0] > maxeffi)
			{
				maxeffi = tran_effi[d0][s0];
				dc_index = d0 + S_Num;
				sate_index = s0;
			}
		}
	}
}

void DelayTime(First_Node*& Node, First_Solution*& FR, int H_Num, int r, int dc, int n)
{//Calculate the allowable delay time for the satellite's arrival
	for (int n0 = 1; n0 < n; n0++)
	{
		int s0 = FR[r].node[n0]; 
		for (int m0 = 1; m0 < H_Num + 1; m0++)
		{
			if (FR[r].supply[s0][m0] == 1 && FR[r].delay[s0] >= Node[s0].at_limit[m0] - FR[r].at[s0])
			{                              

				FR[r].delay[s0] = Node[s0].at_limit[m0] - FR[r].at[s0];
			}
		}
	}
}

void Insert_Node(InsertNode*& InseNode, First_Solution*& FR, int r, int dc, int n, int satellite, int& pos_index, float** First_Nodes_Time)
{//Insert satellite nodes into the first-echelon route
	for (int pos0 = 1; pos0 <= n; pos0++) 
	{		
		int node1 = FR[r].node[pos0 - 1];
		int node2 = FR[r].node[pos0];
		if (pos0 == n)
		{
			node2 = dc;
		}                      
		InseNode[pos0].addtime = First_Nodes_Time[node1][satellite] + First_Nodes_Time[satellite][node2] - First_Nodes_Time[node1][node2];
		for (int pos1 = pos0; pos1 < n; pos1++)
		{
			int node3 = FR[r].node[pos1];
			if (FR[r].delay[node3] < InseNode[pos0].addtime)
			{
				InseNode[pos0].feas = 0;
			}
		}
	}	
	int minaddtime = 100;
	for (int pos0 = 1; pos0 <= n; pos0++) 
	{
		if (InseNode[pos0].feas == 1 && minaddtime >= InseNode[pos0].addtime)
		{
			minaddtime = InseNode[pos0].addtime;
			pos_index = pos0;
		}
	}	
	if (pos_index == n && FR[r].time + minaddtime > T1) 
	{
		pos_index = 0; InseNode[pos_index].feas = 0;
	}
	if (pos_index != 0 && pos_index != n)
	{
		int node1 = FR[r].node[n - 1];
		if (FR[r].time + minaddtime + First_Nodes_Time[node1][dc] > T1)
		{
			pos_index = 0; InseNode[pos_index].feas = 0;
		}
	}
}

void OtherCommodity(int H_Num, int flag_cargo, int sate, First_Solution*& FR, int r, int dc, float**& Sate_Demand, float**& DC_Supply, int** &First_Demand_Node)
{//Determine whether the demand for other commodities at this satellite can be met
	for (int m = 1; m < H_Num + 1; m++)
	{
		if (m != flag_cargo && Sate_Demand[sate][m] <= DC_Supply[dc][m] && FR[r].restC - Sate_Demand[sate][m] > 0 && Sate_Demand[sate][m] != 0)
		{
			FR[r].supply[sate][m] = 1;
			FR[r].restC -= Sate_Demand[sate][m];
			DC_Supply[dc][m] -= Sate_Demand[sate][m];
			int location;
			for (int s0 = 1; s0 < First_Demand_Node[m][0] + 1; s0++)
			{
				if (sate == First_Demand_Node[m][s0])
				{
					location = s0; break;
				}
			}
			for (int s1 = location; s1 < First_Demand_Node[m][0]; s1++)
			{
				First_Demand_Node[m][s1] = First_Demand_Node[m][s1 + 1];                                                            
			}
			First_Demand_Node[m][First_Demand_Node[m][0]] = 0;
			First_Demand_Node[m][0] -= 1;                  
			Sate_Demand[sate][m] = 0;
		}
	}
}

void Update_Route(First_Solution*& FR, int r, int dc, int posi, int satellite, int cargo, float**& Sate_Demand, float**& DC_Supply, float** First_Nodes_Time)
{//update the first-echelon routes	
	FR[r].node[posi] = satellite; 
	FR[r].supply[satellite][cargo] = 1;  
	FR[r].restC -= Sate_Demand[satellite][cargo];	
	FR[r].time += First_Nodes_Time[FR[r].node[posi - 1]][satellite];
	FR[r].at[satellite] = FR[r].at[FR[r].node[posi - 1]] + First_Nodes_Time[FR[r].node[posi - 1]][satellite];
	DC_Supply[dc][cargo] -= Sate_Demand[satellite][cargo];
	Sate_Demand[satellite][cargo] = 0;
}

void Add_Node(int n, int S_Num, int H_Num, First_Node*& Node, First_Solution*& FR, float** First_Nodes_Time, int r, int dc, int& sate1, float**& DC_Supply, float**& Sate_Demand, int* Exist_Nodes, int**& First_Demand_Node)
{//add satellite nodes	
	DelayTime(Node, FR, H_Num, r, dc, n);
	float max_supply = 0;
	for (int s1 = 1; s1 < S_Num + 1; s1++)
	{
		float mm = 0;
		for (int m1 = 1; m1 < H_Num + 1; m1++)
		{
			if (DC_Supply[dc][m1] >= Sate_Demand[s1][m1] && mm <= FR[r].restC)
			{
				mm += Sate_Demand[s1][m1];
			}
		}
		if (max_supply < mm)
		{
			max_supply = mm;
			sate1 = s1;
		}
	}
	if (sate1 != 0)
	{
		int* SP = new int[H_Num + 1];  
		for (int m1 = 0; m1 <= H_Num; m1++)
		{
			if (Sate_Demand[sate1][m1] > 0)
			{
				SP[m1] = m1; SP[0] += 1;
			}
			else SP[m1] = 0;

		}		
		int b;
		for (int m2 = 1; m2 < H_Num; m2++)
		{
			for (int m3 = m2 + 1; m3 <= H_Num; m3++)
			{
				if (Sate_Demand[sate1][SP[m2]] < Sate_Demand[sate1][SP[m3]])
				{
					b = SP[m3];
					SP[m3] = SP[m2];
					SP[m2] = b;          
				}
			}
		}

		int pos_index = 0; 
		Exist_Nodes[sate1] = 0;
		InsertNode* InseNode = new InsertNode[n + 1];                           
		Insert_Node(InseNode, FR, r, dc, n, sate1, pos_index, First_Nodes_Time);	
		if (pos_index != 0) 
		{			
			float temp_at = FR[r].at[FR[r].node[pos_index - 1]] + First_Nodes_Time[FR[r].node[pos_index - 1]][sate1];	
			for (int m1 = 1; m1 <= SP[0]; m1++)
			{
				if (DC_Supply[dc][SP[m1]] >= Sate_Demand[sate1][SP[m1]] && Sate_Demand[sate1][SP[m1]] <= FR[r].restC && temp_at <= Node[sate1].at_limit[SP[m1]])
				{				
					Exist_Nodes[sate1] = 1;                             
					DC_Supply[dc][SP[m1]] -= Sate_Demand[sate1][SP[m1]];
					FR[r].supply[sate1][SP[m1]] = 1; 
					FR[r].restC -= Sate_Demand[sate1][SP[m1]];         
					Sate_Demand[sate1][SP[m1]] = 0;                    
			
					for (int s2 = 1; s2 < First_Demand_Node[SP[m1]][0]; s2++)															 
					{                                                       
																			
						if (sate1 == First_Demand_Node[SP[m1]][s2])
						{                                         
							for (int s3 = s2; s3 < First_Demand_Node[SP[m1]][0]; s3++)
							{                                                         
								First_Demand_Node[SP[m1]][s3] = First_Demand_Node[SP[m1]][s3 + 1];
							}                                                       
																					
							break;
						}          
					}
					
					First_Demand_Node[SP[m1]][First_Demand_Node[SP[m1]][0]] = 0;
					First_Demand_Node[SP[m1]][0] -= 1;					
					SP[m1] = 0;					
				}
			}
		}

		if (Exist_Nodes[sate1] == 1) 
		{			
			for (int nn = n; nn > pos_index; nn--)
			{
				FR[r].node[nn] = FR[r].node[nn - 1];
			}
			FR[r].node[pos_index] = sate1;
			for (int nn = pos_index; nn <= n; nn++)
			{
				FR[r].at[FR[r].node[nn]] = FR[r].at[FR[r].node[nn - 1]] + First_Nodes_Time[FR[r].node[nn - 1]][FR[r].node[nn]];//卡车到达中转仓库的时间
			}
			FR[r].time = FR[r].at[FR[r].node[n]] - FR[r].dt;
		}
		delete[]SP;
		delete[]InseNode;
	}

}

void Calculate_Cargo_Arrive_Time(int S_Num, int H_Num, int BV_Num, First_Node*& Node, First_Solution*& FR)
{//Calculate the time it takes for the commodities to reach the satellites
	for (int u0 = 1; u0 <= S_Num; u0++)         
	{
		for (int m0 = 1; m0 <= H_Num; m0++)      
		{
			for (int r0 = 1; r0 <= BV_Num; r0++)
			{
				if (FR[r0].supply[u0][m0] == 1)
				{
					Node[u0].at[m0] = FR[r0].at[u0];
				}
			}
		}
	}
}

void Initial_First__Route(int D_Num, int S_Num, int H_Num, int BV_Num, First_Node*& Node, First_Solution*& FR, float** First_Distance, float** First_Nodes_Time, int**& cargo_orig)
{//the first-echelon initialization method	
	float** Sate_Demand = new float*[S_Num + 1];
	for (int u1 = 0; u1 < S_Num + 1; u1++)     
	{
		Sate_Demand[u1] = new float[H_Num + 1];
		for (int m1 = 0; m1 < H_Num + 1; m1++) 
		{
			Sate_Demand[u1][m1] = Node[u1].Demand[m1];
		}
	}
	float** DC_Supply = new float*[S_Num + D_Num + 1];
	for (int d1 = 0; d1 < S_Num + D_Num + 1; d1++)
	{
		DC_Supply[d1] = new float[H_Num + 1];
	}
	for (int d1 = S_Num + 1; d1 < S_Num + D_Num + 1; d1++)
	{
		for (int m1 = 0; m1 < H_Num + 1; m1++)
		{
			DC_Supply[d1][m1] = Node[d1].Supply[m1];
		}
	}
	int** First_Demand_Node = new int*[H_Num + 1];  
	for (int m1 = 0; m1 < H_Num + 1; m1++)         
	{
		First_Demand_Node[m1] = new int[S_Num + 1];
		for (int u1 = 0; u1 < S_Num + 1; u1++)     
		{
			First_Demand_Node[m1][u1] = 0;      
		}
	}

	for (int r = 1; r < BV_Num + 1; r++) 
	{
		int unsatisfied_sate = 0;
		Caculate_Nodes_Need(D_Num, S_Num, H_Num, Sate_Demand, First_Demand_Node, unsatisfied_sate);
		if (unsatisfied_sate == 0)
		{
			break;
		}
		int* Exist_Nodes = new int[S_Num + 1]; 
		for (int u1 = 0; u1 < S_Num + 1; u1++)
		{
			Exist_Nodes[u1] = 0;             
		}
		
		int flag_cargo = 0;                   
		for (int m2 = 1; m2 < H_Num + 1; m2++)
		{                                                                
			if (cargo_orig[m2][0] == 1 && First_Demand_Node[m2][0] > 0) 
			{
				flag_cargo = m2; 
				break;
			}
		}
		
		if (flag_cargo != 0)
		{
			int dc = cargo_orig[flag_cargo][1]; 
			int sate = First_Demand_Node[flag_cargo][1]; 
			int index = 1;
		
			for (int s1 = 2; s1 <= First_Demand_Node[flag_cargo][0]; s1++)
			{
				int s2 = First_Demand_Node[flag_cargo][s1];
				if (First_Distance[dc][sate] > First_Distance[dc][s2])
				{
					sate = s2;
					index = s1; 
				}
			}
			
			FR[r].node[1] = sate;
			FR[r].node[0] = dc;
			FR[r].satenum += 1;
			FR[r].dc = dc; 
			FR[r].supply[sate][flag_cargo] = 1;  
			FR[r].restC -= Sate_Demand[sate][flag_cargo];
			FR[r].time += First_Nodes_Time[dc][sate];     		
			FR[r].at[sate] = First_Nodes_Time[dc][sate];

			DC_Supply[dc][flag_cargo] -= Sate_Demand[sate][flag_cargo];
			OtherCommodity(H_Num, flag_cargo, sate, FR, r, dc, Sate_Demand, DC_Supply, First_Demand_Node);
			Exist_Nodes[sate] = 1;
			for (int s1 = index; s1 < First_Demand_Node[flag_cargo][0]; s1++)
			{
				First_Demand_Node[flag_cargo][s1] = First_Demand_Node[flag_cargo][s1 + 1];                                                          
			}
			First_Demand_Node[flag_cargo][First_Demand_Node[flag_cargo][0]] = 0;
			First_Demand_Node[flag_cargo][0] -= 1;                  
			Sate_Demand[sate][flag_cargo] = 0;                      
			
			int temp_n = 2;
			for (int n = 2; n <= S_Num + 1; n++) 
			{				
				int* temp_Sate = new int[First_Demand_Node[flag_cargo][0] + 1];
				temp_Sate[0] = 0;
				for (int s1 = 1; s1 <= First_Demand_Node[flag_cargo][0]; s1++)
				{
					if (Sate_Demand[First_Demand_Node[flag_cargo][s1]][flag_cargo] <= FR[r].restC)
					{
						temp_Sate[0] += 1;                                         
						temp_Sate[temp_Sate[0]] = First_Demand_Node[flag_cargo][s1];
					}
				}
				
				int feasible = 0;     
				if (temp_Sate[0] > 0)
				{					
					int sate1 = temp_Sate[1];                
					int index1 = 1;
					for (int s1 = 2; s1 <= temp_Sate[0]; s1++)
					{
						int s2 = temp_Sate[s1];              
						if (First_Distance[FR[r].node[n - 1]][sate1] > First_Distance[FR[r].node[n - 1]][s2])
						{
							sate1 = s2;
							index1 = s1;
						}
					}

					float Wotk_Time = FR[r].time + First_Nodes_Time[FR[r].node[n - 1]][sate1] + First_Nodes_Time[sate1][dc];
					float at_time = FR[r].at[FR[r].node[n - 1]] + First_Nodes_Time[FR[r].node[n - 1]][sate1];           
					
					if (Wotk_Time <= T1 && at_time <= Node[sate1].at_limit[flag_cargo])
					{
						FR[r].satenum += 1;
						feasible = 1;
						Exist_Nodes[sate1] = 1;
						for (int s1 = index1; s1 < First_Demand_Node[flag_cargo][0]; s1++)
						{
							First_Demand_Node[flag_cargo][s1] = First_Demand_Node[flag_cargo][s1 + 1];
						}
						First_Demand_Node[flag_cargo][First_Demand_Node[flag_cargo][0]] = 0;
						First_Demand_Node[flag_cargo][0] -= 1;
						Update_Route(FR, r, dc, n, sate1, flag_cargo, Sate_Demand, DC_Supply, First_Nodes_Time);
						OtherCommodity(H_Num, flag_cargo, sate1, FR, r, dc, Sate_Demand, DC_Supply, First_Demand_Node);
					}
				}
				delete[]temp_Sate;
				if (feasible == 0)  
				{
					temp_n = n;    
					break;
				}
			}
		
			for (int n5 = temp_n; n5 <= S_Num + 1; n5++)
			{
				int sate1 = 0;
				Add_Node(n5, S_Num, H_Num, Node, FR, First_Nodes_Time, r, dc, sate1, DC_Supply, Sate_Demand, Exist_Nodes, First_Demand_Node);
				FR[r].satenum += 1;
				if (Exist_Nodes[sate1] == 0) 
				{
					FR[r].satenum -= 1;
					FR[r].node[n5] = dc; 
					FR[r].time += First_Nodes_Time[FR[r].node[n5 - 1]][dc];
					break;
				}
			}
		}
		
		else {
			float** tran_effi = new float*[D_Num + 1];  
			for (int d1 = 0; d1 < D_Num + 1; d1++)       
			{
				tran_effi[d1] = new float[S_Num + 1];    
				for (int s1 = 0; s1 < S_Num + 1; s1++)   
				{
					tran_effi[d1][s1] = 0;
				}
			}
			int maxeffi = 0; int dc = 0; int sate = 0;
			Caculate_E(D_Num, S_Num, H_Num, First_Distance, Sate_Demand, DC_Supply, tran_effi, maxeffi, dc, sate);
			
			int* SP = new int[H_Num + 1];  
			for (int m1 = 0; m1 <= H_Num; m1++)
			{
				if (Sate_Demand[sate][m1] > 0)
				{
					SP[m1] = m1; SP[0] += 1;
				}
				else SP[m1] = 0;

			}
			
			int b;
			for (int m2 = 1; m2 < H_Num; m2++)         
			{
				for (int m3 = m2 + 1; m3 <= H_Num; m3++)
				{
					if (Sate_Demand[sate][SP[m2]] < Sate_Demand[sate][SP[m3]])
					{
						b = SP[m3];
						SP[m3] = SP[m2];
						SP[m2] = b;
					}
				}
			}

			for (int m1 = 1; m1 <= SP[0]; m1++)      
			{
				if (DC_Supply[dc][SP[m1]] >= Sate_Demand[sate][SP[m1]] && Sate_Demand[sate][SP[m1]] <= FR[r].restC)
				{
					DC_Supply[dc][SP[m1]] -= Sate_Demand[sate][SP[m1]];
					FR[r].supply[sate][SP[m1]] = 1;  
					FR[r].restC -= Sate_Demand[sate][SP[m1]];
					Sate_Demand[sate][SP[m1]] = 0;   
					SP[m1] = 0;
					for (int s2 = 1; s2 < First_Demand_Node[SP[m1]][0]; s2++)
					{
						if (sate == First_Demand_Node[SP[m1]][s2])
						{
							for (int s3 = s2; s3 < First_Demand_Node[flag_cargo][0]; s3++)
							{
								First_Demand_Node[SP[m1]][s3] = First_Demand_Node[SP[m1]][s3 + 1];
							}
							break;
						}
					}
					First_Demand_Node[flag_cargo][First_Demand_Node[flag_cargo][0]] = 0;
					First_Demand_Node[flag_cargo][0] -= 1;
				}
			}
			
			FR[r].node[1] = sate; 
			FR[r].node[0] = dc;
			FR[r].satenum += 1;
			FR[r].dc = dc; 
			FR[r].time += First_Nodes_Time[dc][sate];
			FR[r].at[sate] = First_Nodes_Time[dc][sate];
			Exist_Nodes[sate] = 1;
			delete[]SP;
				
			for (int n = 2; n <= S_Num + 1; n++) 
			{
				int sate1 = 0;
				FR[r].satenum += 1;
				Add_Node(n, S_Num, H_Num, Node, FR, First_Nodes_Time, r, dc, sate1, DC_Supply, Sate_Demand, Exist_Nodes, First_Demand_Node);
				if (Exist_Nodes[sate1] == 0) 
				{
					FR[r].satenum -= 1;
					FR[r].node[n] = dc; 
					FR[r].time += First_Nodes_Time[FR[r].node[n - 1]][dc];
					break;
				}
			}
		}
	}
	Calculate_Cargo_Arrive_Time(S_Num, H_Num, BV_Num, Node, FR);
}

//########################################## the second-echelon initial solution ##########################################

void Customer_classification(int u, Cust_Set*& CustSet, int S_Num, int C_Num, int H_Num, Second_Cust**& Cust, Second_Solution& SR, First_Solution*& FR, First_Node* Node)
{//Customer classification	
	int* Temp_sate = new int[H_Num + 1];
	for (int m1 = 0; m1 < H_Num + 1; m1++)       
	{
		Temp_sate[m1] = m1;
	}
	for (int m1 = 1; m1 < H_Num; m1++)          
	{
		for (int m2 = m1 + 1; m2 <= H_Num; m2++)
		{
			if (Node[u].at[Temp_sate[m1]] >= Node[u].at[Temp_sate[m2]])
			{
				int temp = Temp_sate[m1];
				Temp_sate[m1] = Temp_sate[m2];
				Temp_sate[m2] = temp;
			}
		}
	}
	
	int**sort_sate = new int*[H_Num + 1];
	for (int m = 0; m <= H_Num; m++)
	{
		sort_sate[m] = new int[H_Num + 1];
		for (int n = 0; n <= H_Num; n++)
		{
			sort_sate[m][n] = 0;
		}
	}
	for (int m = 1; m < H_Num + 1; m++)
	{
		for (int i = 1; i < m; i++)
		{
			if (Node[u].at[Temp_sate[i]] == Node[u].at[Temp_sate[m]])
			{
				m += 1;
				break;
			}
		}
		sort_sate[m][1] = Temp_sate[m];
		sort_sate[m][0] = 1;
		for (int n = 1; n < H_Num + 1; n++)
		{
			if (Node[u].at[Temp_sate[m]] >= Node[u].at[Temp_sate[n]] && n != m)
			{
				sort_sate[m][0] += 1;
				sort_sate[m][sort_sate[m][0]] = Temp_sate[n];
			}
		}
		if (sort_sate[m][0] == H_Num)break;
	}
	for (int m = 1; m < H_Num + 1; m++)
	{
		if (sort_sate[m][0] == 0)
		{
			for (int n = m + 1; n <= H_Num; n++)
			{
				for (int i = 0; i <= H_Num; i++)
				{
					sort_sate[n - 1][i] = sort_sate[n][i];
					sort_sate[n][i] = 0;
				}
			}
		}
	}
	
	for (int m = 1; m < H_Num + 1; m++)
	{
		if (sort_sate[m][0] > 0)
		{
			for (int n = 0; n < H_Num + 1; n++)
			{
				CustSet[m].CargoType[n] = sort_sate[m][n];
			}
			CustSet[m].time = Node[u].at[sort_sate[m][1]];


			for (int i = 1; i <= C_Num; i++)        
			{
				int feas = 1; 
				for (int m2 = 1; m2 <= H_Num; m2++)  
				{
					if (Cust[u][i].Demand[m2] > 0 && Node[u].at[m2] > CustSet[m].time)
					{
						feas = 0;
						break;
					}
				}
				if (feas == 1)
				{
					CustSet[m].cust[0] += 1;
					CustSet[m].cust[CustSet[m].cust[0]] = i;
				}
			}
		}

	}	
}

void Second_Route_Adjust_Time(int u, int r, int i, float**& AT, float**& BT, Second_Cust**& Cust, Second_Solution& SR, float cargoArriveTime)
{// Adjust the departure time of the electric vehicle
	AT[r][0] = SR.dt[u][r] - cargoArriveTime - tope;
	BT[r][0] = 8;
	for (int i0 = 1; i0 < i; i0++)                      
	{
		int j = SR.node[u][r][i0];                      
		AT[r][j] = SR.at[u][r][j] - Cust[u][j].Earliest_ST;
		BT[r][j] = Cust[u][j].Latest_ST - SR.at[u][r][j];  
		if (AT[r][0] >= AT[r][j])
		{
			AT[r][0] = AT[r][j];
		}
		if (BT[r][0] >= BT[r][j])
		{
			BT[r][0] = BT[r][j];
		}
	}
}

void Second_Route_Cust_Update_Time(int u, int r, int i, int pre_cust, int cur_cust, float**& AT, float**& BT, Second_Cust**& Cust, Second_Solution& SR, float*** Second_Cust_Time)
{//Update the customer arrival times in a route
	if (SR.at[u][r][pre_cust] + Second_Cust_Time[u][pre_cust][cur_cust] > Cust[u][cur_cust].Latest_ST)
	{
		float AT = SR.at[u][r][pre_cust] + Second_Cust_Time[u][pre_cust][cur_cust] - Cust[u][cur_cust].Latest_ST;
		SR.dt[u][r] -= AT;             
		for (int i0 = 1; i0 < i; i0++)
		{
			int j = SR.node[u][r][i0];
			SR.at[u][r][j] -= AT;
		}
		SR.at[u][r][cur_cust] = Cust[u][cur_cust].Latest_ST;
	}
	if (SR.at[u][r][pre_cust] + Second_Cust_Time[u][pre_cust][cur_cust] <= Cust[u][cur_cust].Earliest_ST)
	{
		float BT = Cust[u][cur_cust].Earliest_ST - SR.at[u][r][pre_cust] - Second_Cust_Time[u][pre_cust][cur_cust];
		SR.dt[u][r] += BT;             
		for (int i0 = 1; i0 < i; i0++)
		{
			int j = SR.node[u][r][i0];
			SR.at[u][r][j] += BT;
		}
		SR.at[u][r][cur_cust] = Cust[u][cur_cust].Earliest_ST;
	}
}

void Update_Cust_Set(int u, int m, int H_Num, int index, int cust, int **&Cust_list)
{//Update the customer list	
	for (int ii = index; ii < Cust_list[m][0]; ii++)
	{
		Cust_list[m][ii] = Cust_list[m][ii + 1];
	}
	Cust_list[m][Cust_list[m][0]] = 0;
	Cust_list[m][0] -= 1;

	for (int m1 = 1; m1 <= H_Num; m1++)
	{
		if (Cust_list[m1][0] > 0 && m1 != m)
		{
			for (int i = 1; i <= Cust_list[m1][0]; i++)
			{
				if (Cust_list[m1][i] == cust)
				{
					for (int ii = i; ii < Cust_list[m1][0]; ii++)
					{
						Cust_list[m1][ii] = Cust_list[m1][ii + 1];
					}
					Cust_list[m1][Cust_list[m1][0]] = 0;
					Cust_list[m1][0] -= 1;
					break;
				}
			}
		}
	}
}

void Judge_j(int u, int r, int &j, int C_Num, Second_Solution SR)
{//Determine the route
	for (int i = 1; i < C_Num + 1; i++)
	{
		if (SR.node[u][r][i] == 0)
		{
			j = i;
			break;
		}
	}
}

void Initial_Second_Route(int S_Num, int C_Num, int H_Num, int SV_Num, First_Node* Node, First_Solution*& FR, float**& First_Nodes_Time, Second_Cust**& Cust, Second_Solution& SR, float*** Second_Cust_Time)
{//the second-echelon initialization method
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
	
	for (int u = 1; u < S_Num + 1; u++)    
	{
		for (int i = 1; i < C_Num + 1; i++)
		{
			SR.Cust_Demand[u][i] = Cust[u][i].Total_Demand;
		}
	}
	for (int u = 1; u < S_Num + 1; u++)              
	{//The criteria for judgment here are arbitrarily set.
		if (C_Num < 30)//The second-echelon initial solution construction based on waiting time
		{			
			int *nodeNum = new int[SV_Num + 1];
			for (int r = 0; r < SV_Num + 1; r++)nodeNum[r] = 0;			
			float **sche_dt = new float*[C_Num + 1]; sche_dt[0] = 0;
			for (int c = 1; c < C_Num + 1; c++)
			{
				sche_dt[c] = new float[2];
				sche_dt[c][0] = c;
				sche_dt[c][1] = Cust[u][c].Earliest_ST - Second_Cust_Time[u][0][c];
			}
			for (int c = 1; c < C_Num; c++)
			{
				int index = 0;
				float comparison = sche_dt[c][1]; int com_number = sche_dt[c][0];
				for (int c1 = c + 1; c1 < C_Num + 1; c1++)
				{
					if (comparison > sche_dt[c1][1])
					{
						comparison = sche_dt[c1][1];
						com_number = sche_dt[c1][0];
						index = c1;
					}
				}				
				if (index > 0)
				{
					sche_dt[index][0] = sche_dt[c][0];
					sche_dt[index][1] = sche_dt[c][1];
					sche_dt[c][0] = com_number;
					sche_dt[c][1] = comparison;
				}
				
			}
			int *unsatis_cust = new int[C_Num + 1]; unsatis_cust[0] = 10;
			for (int c = 1; c < C_Num + 1; c++)unsatis_cust[c] = c;
			float** AT = new float*[SV_Num + 1];
			float** BT = new float*[SV_Num + 1];
			for (int r1 = 1; r1 < SV_Num + 1; r1++)
			{
				AT[r1] = new float[C_Num + 1];
				BT[r1] = new float[C_Num + 1];
				for (int i1 = 0; i1 < C_Num + 1; i1++)//i1=0 1 2 3 4 5 6 7 8 9 10
				{
					AT[r1][i1] = 8;
					BT[r1][i1] = 8;
				}
			}
			
			SR.node[u][1][1] = sche_dt[1][0]; int cust1 = sche_dt[1][0]; unsatis_cust[cust1] = 0;	
			SR.node[u][2][1] = sche_dt[2][0]; int cust2 = sche_dt[2][0]; unsatis_cust[cust2] = 0;
			unsatis_cust[0] -= 2;			
			for (int r = 1; r < 3; r++)
			{
				int fir_cust = SR.node[u][r][1]; nodeNum[r] = 1;
				if (r == 1)
				{
					SR.at[u][r][fir_cust] = (Cust[u][fir_cust].Earliest_ST + Cust[u][fir_cust].Latest_ST) / 2;
					SR.dt[u][r] = SR.at[u][r][fir_cust] - Second_Cust_Time[u][0][fir_cust];
				}
				if (r == 2)
				{
					if (SR.dt[u][1] + Second_Cust_Time[u][0][fir_cust] < Cust[u][fir_cust].Earliest_ST)SR.at[u][r][fir_cust] = Cust[u][fir_cust].Earliest_ST;
					if (SR.dt[u][1] + Second_Cust_Time[u][0][fir_cust] >= Cust[u][fir_cust].Earliest_ST
						&&SR.dt[u][1] + Second_Cust_Time[u][0][fir_cust] <= Cust[u][fir_cust].Latest_ST)SR.at[u][r][fir_cust] = SR.dt[u][1] + Second_Cust_Time[u][0][fir_cust];
					if (SR.dt[u][1] + Second_Cust_Time[u][0][fir_cust] > Cust[u][fir_cust].Latest_ST)SR.at[u][r][fir_cust] = Cust[u][fir_cust].Latest_ST;
				}
				AT[r][1] = SR.at[u][r][fir_cust] - Cust[u][fir_cust].Earliest_ST;
				BT[r][1] = Cust[u][fir_cust].Latest_ST - SR.at[u][r][fir_cust];
				AT[r][0] = AT[r][1]; BT[r][0] = BT[r][1];
				SR.time[u][r] = Second_Cust_Time[u][0][fir_cust];
				SR.restC[u][r] -= Cust[u][fir_cust].Total_Demand;				
				for (int j = 2; j < C_Num + 1; j++)
				{

					if (SR.time[u][r] > T2 || SR.restC < 0)break;
					int pre_cust = SR.node[u][r][j - 1];
					float min_time = 1000; int guid_cust = 0;
					
					for (int i = 1; i < C_Num + 1; i++)
					{
						if (unsatis_cust[i] != 0)
						{							
							if ((SR.at[u][r][pre_cust] + Second_Cust_Time[u][pre_cust][i] + BT[r][0] >= Cust[u][i].Earliest_ST)
								&& (SR.at[u][r][pre_cust] + Second_Cust_Time[u][pre_cust][i] - AT[r][0] <= Cust[u][i].Latest_ST))
							{								
								if (min_time > Second_Cust_Time[u][pre_cust][i])
								{
									min_time = Second_Cust_Time[u][pre_cust][i];
									guid_cust = i;
								}								
							}
						}
					}
					
					if (guid_cust != 0)
					{
						SR.node[u][r][j] = guid_cust;
						unsatis_cust[guid_cust] = 0;
						unsatis_cust[0] -= 1;
						SR.time[u][r] += min_time;
						SR.restC[u][r] -= Cust[u][guid_cust].Total_Demand;						
						SR.at[u][r][guid_cust] = SR.at[u][r][pre_cust] + Second_Cust_Time[u][pre_cust][guid_cust];
						Second_Route_Cust_Update_Time(u, r, j, pre_cust, guid_cust, AT, BT, Cust, SR, Second_Cust_Time);						
						float max_time = Node[u].at[1];
						for (int m = 2; m < H_Num + 1; m++)
						{
							if (max_time < Node[u].at[m])max_time = Node[u].at[m];
						}
						Second_Route_Adjust_Time(u, r, j + 1, AT, BT, Cust, SR, max_time);						
						nodeNum[r] = j;

					}
					else
					{
						break;
					}
				}
				SR.dt[u][r] = SR.at[u][r][fir_cust] - Second_Cust_Time[u][0][fir_cust];
			}

			if (unsatis_cust[0] > 0)
			{				
				for (int i = 1; i < C_Num + 1; i++)
				{
					if (unsatis_cust[i] > 0)
					{
						int index = 0; int car_index = 0;
						for (int r = 1; r < 3; r++)
						{
							if (SR.restC[u][r] - Cust[u][i].Total_Demand >= 0)
							{
								for (int j = 1; j < nodeNum[r]; j++)
								{
									if (index > 0)break;
									int pre_cust = SR.node[u][r][j]; int lat_cust = SR.node[u][r][j + 1];
									if ((SR.at[u][r][pre_cust] + Second_Cust_Time[u][pre_cust][i] + BT[r][0] >= Cust[u][i].Earliest_ST) &&
										(SR.at[u][r][pre_cust] + Second_Cust_Time[u][pre_cust][i] - AT[r][0] <= Cust[u][i].Latest_ST))
									{
										int count = 0;
										float add_time = Second_Cust_Time[u][pre_cust][i] + Second_Cust_Time[u][i][lat_cust];
										if (SR.time[u][r] + add_time <= T2)
										{
											for (int j2 = j + 1; j2 < nodeNum[r] + 1; j2++)
											{
												if ((SR.at[u][r][SR.node[u][r][j2]] + add_time + BT[r][0] >= Cust[u][SR.node[u][r][j2]].Earliest_ST) &&
													(SR.at[u][r][SR.node[u][r][j2]] + add_time - AT[r][0] <= Cust[u][SR.node[u][r][j2]].Latest_ST))
												{
													count += 1;
												}
											}
										}
										if (count == nodeNum[r] - j)
										{
											index = j; car_index = r;
										}
									}

								}
							}
						}
						if (index > 0)
						{
							int r = car_index;
							unsatis_cust[0] -= 1;
							unsatis_cust[i] = 0;							
							for (int j = nodeNum[r] + 1; j > index; j--)
							{
								SR.node[u][r][j] = SR.node[u][r][j - 1];
							}
							SR.node[u][r][index] = i;
							nodeNum[r] += 1;
							SR.restC[u][r] -= Cust[u][i].Total_Demand;
							for (int j = index; j < nodeNum[r] + 1; j++)
							{
								int pre_cust = SR.node[u][r][j - 1];
								SR.at[u][r][SR.node[u][r][j]] = SR.at[u][r][pre_cust] + Second_Cust_Time[u][pre_cust][SR.node[u][r][j]];
								float max_time = Node[u].at[1];
								for (int m = 2; m < H_Num + 1; m++)
								{
									if (max_time < Node[u].at[m])max_time = Node[u].at[m];
								}
								Second_Route_Adjust_Time(u, r, j, AT, BT, Cust, SR, max_time);
								Second_Route_Cust_Update_Time(u, r, j, pre_cust, SR.node[u][r][j], AT, BT, Cust, SR, Second_Cust_Time);
							}
							SR.time[u][r] = SR.at[u][r][SR.node[u][r][nodeNum[r]]] + Second_Cust_Time[u][0][SR.node[u][r][1]] + Second_Cust_Time[u][SR.node[u][r][nodeNum[r]]][0];
							SR.dt[u][r] = SR.at[u][r][SR.node[u][r][1]] - Second_Cust_Time[u][0][SR.node[u][r][1]];
						}
					}
				}
				
			}
			
			for (int r = 1; r < SV_Num + 1; r++)
			{
				if (nodeNum[r] > 0)
				{
					for (int m = 1; m < H_Num + 1; m++)
					{
						SR.load[u][r][m] = 0;
					}
					for (int i = 1; i < nodeNum[r] + 1; i++)
					{
						int cust = SR.node[u][r][i];
						for (int m = 1; m < H_Num + 1; m++)
						{
							if (Cust[u][cust].Demand[m] > 0)
							{
								SR.load[u][r][m] = 1;
							}
						}
					}					
				}

			}
		}
		else//The second-echelon initial solution construction based on customer time window
		{
			Cust_Set* CustSet = new Cust_Set[H_Num + 1];
			Customer_classification(u, CustSet, S_Num, C_Num, H_Num, Cust, SR, FR, Node);

			int** Cust_list = new int*[H_Num + 1];
			for (int m1 = 0; m1 < H_Num + 1; m1++)      
			{
				Cust_list[m1] = new int[C_Num + 1];
				for (int c1 = 0; c1 < C_Num + 1; c1++) 
				{
					Cust_list[m1][c1] = CustSet[m1].cust[c1];			
				}
			}		
			float** AT = new float*[SV_Num + 1];     
			float** BT = new float*[SV_Num + 1];
			for (int r1 = 1; r1 < SV_Num + 1; r1++)   
			{
				AT[r1] = new float[C_Num + 1];
				BT[r1] = new float[C_Num + 1];
				for (int i1 = 0; i1 < C_Num + 1; i1++)
				{
					AT[r1][i1] = 8;
					BT[r1][i1] = 8;
				}
			}
			for (int r = 1; r < SV_Num + 1; r++)       
			{
				for (int m2 = 1; m2 < H_Num + 1; m2++)
				{
					if (Cust_list[m2][0] > 0)
					{
						int j;
						Judge_j(u, r, j, C_Num, SR);
						for (int i = j; i < C_Num + 1; i++)                    
						{   
							for (int h = 1; h <= CustSet[m2].CargoType[0]; h++)
							{
								SR.load[u][r][CustSet[m2].CargoType[h]] = 1;
							}
							if (Cust_list[m2][0] != 0)                    
							{
								if (i == 1)                              
								{
									
									for (int p = 1; p < Cust_list[m2][0]; p++)            
									{
										for (int q = p + 1; q < Cust_list[m2][0] + 1; q++)
										{
											if (Cust[u][Cust_list[m2][p]].Latest_ST > Cust[u][Cust_list[m2][q]].Latest_ST)
											{
												int a = Cust_list[m2][q];
												Cust_list[m2][q] = Cust_list[m2][p];
												Cust_list[m2][p] = a;
											}
										}
									}
									
									SR.node[u][r][i] = Cust_list[m2][1];//i=1
									int visi_cust = SR.node[u][r][i];								
									SR.dt[u][r] = Cust[u][visi_cust].Latest_ST - Second_Cust_Time[u][0][visi_cust];
									SR.at[u][r][0] = SR.dt[u][r];
									SR.at[u][r][visi_cust] = Cust[u][visi_cust].Latest_ST;								
									SR.restC[u][r] -= SR.Cust_Demand[u][visi_cust];
									SR.time[u][r] += Second_Cust_Time[u][0][visi_cust];									
									SR.Cust_Demand[u][visi_cust] = 0;
									Update_Cust_Set(u, m2, H_Num, 1, Cust_list[m2][1], Cust_list);
								}
								else
								{
									for (int c = 0; c < C_Num + 1; c++)
									{
										AT[r][c] = 8; BT[r][c] = 8;
									}
									float cargoArriveTime = CustSet[m2].time;
									Second_Route_Adjust_Time(u, r, i, AT, BT, Cust, SR, cargoArriveTime);									
									int b;
									int pre_cust = SR.node[u][r][i - 1];    
									
									for (int p = 1; p < Cust_list[m2][0]; p++)
									{
										for (int q = p + 1; q < Cust_list[m2][0] + 1; q++)
										{
											if (Second_Cust_Time[u][pre_cust][Cust_list[m2][p]] > Second_Cust_Time[u][pre_cust][Cust_list[m2][q]])
											{
												b = Cust_list[m2][q];
												Cust_list[m2][q] = Cust_list[m2][p];
												Cust_list[m2][p] = b;
											}
										}
									}
									
									for (int s = 1; s < Cust_list[m2][0] + 1; s++)
									{
										
										if (pre_cust != 0) 
										{
											int cur_cust = Cust_list[m2][s];											
												
											if ((SR.time[u][r] + Second_Cust_Time[u][pre_cust][cur_cust] + Second_Cust_Time[u][cur_cust][0] <= T2)
												&& (SR.at[u][r][pre_cust] + Second_Cust_Time[u][pre_cust][cur_cust] - AT[r][0] <= Cust[u][cur_cust].Latest_ST)
												&& (SR.at[u][r][pre_cust] + Second_Cust_Time[u][pre_cust][cur_cust] + BT[r][0] >= Cust[u][cur_cust].Earliest_ST)
												&& (SR.Cust_Demand[u][cur_cust] <= SR.restC[u][r]))
											{
												SR.node[u][r][i] = cur_cust;												
												Second_Route_Cust_Update_Time(u, r, i, pre_cust, cur_cust, AT, BT, Cust, SR, Second_Cust_Time);												
												SR.restC[u][r] -= SR.Cust_Demand[u][cur_cust];
												SR.time[u][r] += Second_Cust_Time[u][pre_cust][cur_cust];												
												SR.Cust_Demand[u][cur_cust] = 0;
												SR.at[u][r][cur_cust] = SR.at[u][r][pre_cust] + Second_Cust_Time[u][pre_cust][cur_cust];
												Update_Cust_Set(u, m2, H_Num, s, cur_cust, Cust_list);
												break;
											}
										}
									}									
								}
							}
							else
							{			
								break;  
							}
						}
					}
				}
			}
		}

	}	
	for (int u = 1; u < S_Num + 1; u++)        
	{
		for (int r = 1; r < SV_Num + 1; r++)  
		{
			for (int i = 1; i < C_Num + 1; i++)
			{
				if (SR.node[u][r][i] == 0 && SR.node[u][r][i - 1] != 0)
				{
					SR.time[u][r] += Second_Cust_Time[u][i - 1][0];
				}
			}
		}
	}	
}
