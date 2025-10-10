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

//***************************************** The second-echelon perturbation operators ************************************

void Cargo_Arrive_Time(int u, int r, int nodeNum, float& cargoArriveTime, First_Node* Node, Second_Cust**& Cust, Second_Solution& SR)
{//Delivery time of the commodities
	for (int m = 1; m < H_Num + 1; m++)
	{
		SR.load[u][r][m] = 0;
	}
	for (int i = 1; i < nodeNum + 1; i++)
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
	for (int m = 1; m < H_Num + 1; m++)
	{
		if (SR.load[u][r][m] == 1)
		{
			if (cargoArriveTime < Node[u].at[m])
			{
				cargoArriveTime = Node[u].at[m];
			}
		}
	}
}

void ExchangeOperator01(int u, int**& routeNum, First_Node* Node, Second_Solution& SR, Second_Cust**& Cust, float*** Second_Cust_Time, float***& Second_Cust_Cost, float** &AT, float**&BT, Second_Solution & newSR)
{//the (0,1) inter-route exchange operator
	int route01 = rand() % routeNum[0][0] + 1;
	int r = routeNum[route01][0];
	int nodeNum = routeNum[route01][1];
	int index01 = rand() % nodeNum + 1;
	int removalCust = SR.node[u][r][index01];
	int feas = 0;
	if (index01 == 1 || index01 == nodeNum)
	{
		feas = 1;
	}
	else
	{
		int pre_cust = SR.node[u][r][index01 - 1];
		int lat_cust = SR.node[u][r][index01 + 1];
		float changeTime = Second_Cust_Time[u][pre_cust][removalCust] + Second_Cust_Time[u][removalCust][lat_cust] - Second_Cust_Time[u][pre_cust][lat_cust];
		for (int i = index01; i < nodeNum; i++)
		{
			int cust = newSR.node[u][r][i + 1];
			newSR.node[u][r][i] = cust;
			newSR.at[u][r][cust] -= changeTime;
		}
		float cargoArriveTime = 0.0;
		Cargo_Arrive_Time(u, r, nodeNum, cargoArriveTime, Node, Cust, newSR);
		Second_Route_Adjust_Time(u, r, nodeNum, AT, BT, Cust, newSR, cargoArriveTime);
		if (AT[r][0] > 0 && BT[r][0] > 0)
		{
			feas = 1;
		}
	}
	InsertNode * routeInsert = new InsertNode[routeNum[0][0] + 1];
	if (feas == 1)
	{
		for (int r1 = 1; r1 < routeNum[0][0] + 1; r1++)
		{
			if (r1 != route01)
			{
				float addTime = 10000.0;
				int index = 0;
				int k = routeNum[r1][0];
				float time = 0.0;
				Cargo_Arrive_Time(u, k, routeNum[r1][1], time, Node, Cust, newSR);
				Second_Route_Adjust_Time(u, k, routeNum[r1][1] + 1, AT, BT, Cust, newSR, time);
				for (int i = 1; i < routeNum[r1][1] + 2; i++)
				{
					if (i == 1)
					{
						int cust = newSR.node[u][k][1];
						float at_time = newSR.at[u][k][cust] - Second_Cust_Time[u][cust][removalCust];
						if (at_time - AT[k][0] <= Cust[u][removalCust].Latest_ST
							&&at_time + BT[k][0] >= Cust[u][removalCust].Earliest_ST
							&&newSR.time[u][k] + Second_Cust_Time[u][0][removalCust] + Second_Cust_Time[u][removalCust][cust] - Second_Cust_Time[u][0][cust] <= T2)
						{
							if (addTime > Second_Cust_Time[u][0][removalCust] + Second_Cust_Time[u][removalCust][cust] - Second_Cust_Time[u][0][cust] - at_time)
							{
								addTime = Second_Cust_Time[u][0][removalCust] + Second_Cust_Time[u][removalCust][cust] - Second_Cust_Time[u][0][cust];
								index = i;
								addTime -= at_time;
							}
						}
					}
					if (i == routeNum[r1][1] + 1)
					{
						int cust = newSR.node[u][k][routeNum[r1][1]];
						float at_time = newSR.at[u][k][cust] + Second_Cust_Time[u][cust][removalCust];
						if (at_time - AT[k][0] <= Cust[u][removalCust].Latest_ST
							&&at_time + BT[k][0] >= Cust[u][removalCust].Earliest_ST
							&&newSR.time[u][k] + Second_Cust_Time[u][0][removalCust] + Second_Cust_Time[u][removalCust][cust] - Second_Cust_Time[u][0][cust] <= T2)
						{
							if (addTime > Second_Cust_Time[u][0][removalCust] + Second_Cust_Time[u][removalCust][cust] - Second_Cust_Time[u][0][cust])
							{
								addTime = Second_Cust_Time[u][0][removalCust] + Second_Cust_Time[u][removalCust][cust] - Second_Cust_Time[u][0][cust];
								index = i;
							}
						}
					}
					if (i > 1 && i < routeNum[r1][1] + 1)
					{
						int pre_cust = newSR.node[u][k][i - 1];
						int cust = newSR.node[u][k][i];
						float at_time = newSR.at[u][k][pre_cust] + Second_Cust_Time[u][pre_cust][removalCust];
						float changeTime = Second_Cust_Time[u][cust][removalCust] + Second_Cust_Time[u][removalCust][pre_cust] - Second_Cust_Time[u][pre_cust][cust];
						if (at_time - AT[k][0] <= Cust[u][removalCust].Latest_ST
							&&at_time + BT[k][0] >= Cust[u][removalCust].Earliest_ST
							&& newSR.time[u][k] + changeTime <= T2)
						{
							int ok = 1;
							float **AT1 = new float*[SV_Num + 1];
							float **BT1 = new float*[SV_Num + 1];
							for (int r2 = 0; r2 < SV_Num + 1; r2++)
							{
								AT1[r2] = new float[C_Num + 1];
								BT1[r2] = new float[C_Num + 1];
								for (int c = 0; c < C_Num + 1; c++)
								{
									AT1[r2][c] = 8;
								}
							}
							AT1[k][0] = AT[k][0];
							BT1[k][0] = BT[k][0];
							AT1[k][removalCust] = at_time - Cust[u][removalCust].Earliest_ST;
							BT1[k][removalCust] = Cust[u][removalCust].Latest_ST - at_time;
							if (AT1[k][0] >= AT1[k][removalCust])
							{
								AT1[k][0] = AT1[k][removalCust];
							}
							if (BT1[k][0] >= BT1[k][removalCust])
							{
								BT1[k][0] = BT1[k][removalCust];
							}
							for (int i1 = i; i1 < routeNum[r1][1] + 1; i1++)
							{
								int cust1 = newSR.node[u][k][i1];
								if (newSR.at[u][k][cust1] + changeTime - AT1[k][0] >= Cust[u][cust1].Latest_ST
									|| newSR.at[u][k][cust1] + changeTime + BT1[k][0] <= Cust[u][cust1].Earliest_ST)
								{
									ok = 0;
									break;
								}
								else
								{
									if ((newSR.at[u][k][cust1] + changeTime) - Cust[u][cust1].Earliest_ST < AT1[k][0])
									{
										AT1[k][0] = (newSR.at[u][k][cust1] + changeTime) - Cust[u][cust1].Earliest_ST;
									}
									if (Cust[u][cust1].Latest_ST - (newSR.at[u][k][cust1] + changeTime) < BT1[k][0])
									{
										BT1[k][0] = Cust[u][cust1].Latest_ST - (newSR.at[u][k][cust1] + changeTime);
									}
								}
							}
							if (ok == 1)
							{
								index = i;
								float adjust_time = AT[k][0] - AT1[k][0];
								if (addTime > changeTime - adjust_time)
									addTime = changeTime - adjust_time;
							}
							delete[] AT1;
							delete[] BT1;
						}
					}
				}
				if (index != 0)
				{
					routeInsert[r1].car = r1;
					routeInsert[r1].pos = index;
					routeInsert[r1].addtime = addTime;
				}
			}
		}
		int finalRoute = 1;
		for (int r1 = 1; r1 < routeNum[0][0] + 1; r1++)
		{
			if (routeInsert[finalRoute].addtime > routeInsert[r1].addtime)
			{
				finalRoute = r1;
			}
		}
		int index02 = routeInsert[finalRoute].pos;
		if (index02 > 0)
		{
			SR.at[u][r][removalCust] = 0;
			SR.restC[u][r] += Cust[u][removalCust].Total_Demand;
			if (index01 == nodeNum)
			{
				SR.node[u][r][nodeNum] = 0;
			}
			else
			{
				for (int i = index01; i < nodeNum; i++)
				{
					SR.node[u][r][i] = SR.node[u][r][i + 1];
				}
				SR.node[u][r][nodeNum] = 0;
				float cargoArriveTime = 0.0;
				Cargo_Arrive_Time(u, r, nodeNum - 1, cargoArriveTime, Node, Cust, SR);
				for (int i = index01; i < nodeNum; i++)
				{
					if (index01 != 1)
					{
						int cust = SR.node[u][r][i];
						int pre_cust = SR.node[u][r][i - 1];
						SR.at[u][r][cust] = SR.at[u][r][pre_cust] + Second_Cust_Time[u][pre_cust][cust];
						for (int c = 0; c < C_Num + 1; c++)
						{
							AT[r][c] = 8; BT[r][c] = 8;
						}
						Second_Route_Adjust_Time(u, r, i, AT, BT, Cust, SR, cargoArriveTime);
						Second_Route_Cust_Update_Time(u, r, i, pre_cust, cust, AT, BT, Cust, SR, Second_Cust_Time);
					}
				}
			}
			routeNum[route01][1] -= 1;
			int fir_cust = SR.node[u][r][1];
			SR.dt[u][r] = SR.at[u][r][fir_cust] - Second_Cust_Time[u][0][fir_cust];
			int las_cust01 = SR.node[u][r][routeNum[route01][1]];
			SR.time[u][r] = SR.at[u][r][las_cust01] + Second_Cust_Time[u][las_cust01][0];

			int route02 = routeInsert[finalRoute].car;
			int k = routeNum[route02][0];
			int nodeNum02 = routeNum[route02][1];
			float addTime = routeInsert[finalRoute].addtime;
			for (int i = nodeNum02 + 1; i > index02; i--)
			{
				SR.node[u][k][i] = SR.node[u][k][i - 1];
			}
			SR.node[u][k][index02] = removalCust;
			routeNum[route02][1] += 1;
			if (index02 == 1)
			{
				int lat_cust = SR.node[u][k][index02 + 1];
				SR.at[u][k][removalCust] = SR.at[u][k][lat_cust] - Second_Cust_Time[u][removalCust][lat_cust];
				SR.dt[u][k] = SR.at[u][k][removalCust] - Second_Cust_Time[u][0][removalCust];
			}
			if (index02 == routeNum[route02][1])
			{
				int pre_cust = SR.node[u][k][index02 - 1];
				SR.at[u][k][removalCust] = SR.at[u][k][pre_cust] + Second_Cust_Time[u][pre_cust][removalCust];
			}
			if (index02 > 1 && index02 < routeNum[route02][1])
			{
				float cargoArriveTime = 0.0;
				Cargo_Arrive_Time(u, k, routeNum[route02][1], cargoArriveTime, Node, Cust, SR);
				for (int i = index02; i < routeNum[route02][1] + 1; i++)
				{
					int cust = SR.node[u][k][i];
					int pre_cust = SR.node[u][k][i - 1];
					SR.at[u][k][cust] = SR.at[u][k][pre_cust] + Second_Cust_Time[u][pre_cust][cust];
					for (int c = 0; c < C_Num + 1; c++)
					{
						AT[k][c] = 8; BT[k][c] = 8;
					}
					Second_Route_Adjust_Time(u, k, i, AT, BT, Cust, SR, cargoArriveTime);
					Second_Route_Cust_Update_Time(u, k, i, pre_cust, cust, AT, BT, Cust, SR, Second_Cust_Time);
				}
			}
			int las_cust02 = SR.node[u][k][routeNum[route02][1]];
			SR.time[u][k] = SR.at[u][k][las_cust02] + Second_Cust_Time[u][las_cust02][0];
			SR.restC[u][k] -= Cust[u][removalCust].Total_Demand;
		}
	}
}

void ExchangeOperator02(int u, int**& routeNum, First_Node* Node, Second_Cust**& Cust, Second_Solution& SR, float*** Second_Cust_Time, Second_Solution & newSR, float** &AT, float**&BT)
{//the (1,1) intra-route exchange operator
	int route = rand() % routeNum[0][0] + 1;
	int index1 = rand() % routeNum[route][1] + 1;
	int nodeNum = routeNum[route][1];
	int r = routeNum[route][0];
	int exchangeCust = SR.node[u][r][index1];
	int *feas = new int[nodeNum + 1];
	float minTime = 10000.0;
	int minIndex = 0;
	float cargoArriveTime = 0.0;
	Cargo_Arrive_Time(u, r, nodeNum, cargoArriveTime, Node, Cust, SR);
	Second_Route_Adjust_Time(u, r, routeNum[r][1] + 1, AT, BT, Cust, SR, cargoArriveTime);
	float AT0 = AT[r][0];
	float BT0 = BT[r][0];
	for (int i = 1; i < nodeNum + 1; i++)
	{
		feas[i] = 0;
	}
	for (int i = 1; i < nodeNum + 1; i++)
	{
		if (i != index1)
		{
			int otherCust = SR.node[u][r][i];
			float at_time1 = SR.at[u][r][SR.node[u][r][i - 1]] + Second_Cust_Time[u][SR.node[u][r][i - 1]][exchangeCust];
			float at_time2 = SR.at[u][r][SR.node[u][r][index1 - 1]] + Second_Cust_Time[u][SR.node[u][r][index1 - 1]][otherCust];
			newSR = SR;
			if (at_time1 <= Cust[u][exchangeCust].Latest_ST && at_time1 >= Cust[u][exchangeCust].Earliest_ST
				&& at_time2 <= Cust[u][otherCust].Latest_ST && at_time2 >= Cust[u][otherCust].Earliest_ST)
			{
				newSR.node[u][r][index1] = otherCust;
				newSR.node[u][r][i] = exchangeCust;
				newSR.at[u][r][exchangeCust] = at_time1;
				newSR.at[u][r][otherCust] = at_time2;
				feas[i] = 1;
				for (int j = 1; j < nodeNum + 1; j++)
				{
					int pre_cust = newSR.node[u][r][j - 1];
					int cur_cust = newSR.node[u][r][j];
					Second_Route_Adjust_Time(u, r, j, AT, BT, Cust, newSR, cargoArriveTime);
					newSR.at[u][r][cur_cust] = newSR.at[u][r][pre_cust] + Second_Cust_Time[u][pre_cust][cur_cust];
					if (AT[r][0] < 0 || BT[r][0] < 0)
					{
						feas[i] = 0;
						break;
					}
					else
					{
						if (newSR.at[u][r][cur_cust] - AT[r][0] <= Cust[u][cur_cust].Latest_ST
							&& newSR.at[u][r][cur_cust] + BT[r][0] >= Cust[u][cur_cust].Earliest_ST)
						{
							Second_Route_Cust_Update_Time(u, r, j, pre_cust, cur_cust, AT, BT, Cust, newSR, Second_Cust_Time);
						}
						else
						{
							feas[i] = 0;
							break;
						}
					}
				}
			}
			if (feas[i] == 1)
			{
				float totalTime = 0.0;
				for (int j = 0; j < nodeNum + 1; j++)
				{
					totalTime += Second_Cust_Time[u][newSR.node[u][r][j]][newSR.node[u][r][j + 1]];
				}
				float adjust_time = AT[r][0] - AT0;
				totalTime = totalTime - adjust_time;
				if (totalTime <= minTime)
				{
					minTime = totalTime;
					minIndex = i;
				}
			}
		}
	}
	if (minIndex > 0)
	{
		int OtherCust = SR.node[u][r][minIndex];
		SR.node[u][r][minIndex] = exchangeCust;
		SR.node[u][r][index1] = OtherCust;
		SR.time[u][r] = minTime;
		for (int j = 1; j < nodeNum + 1; j++)
		{
			int pre_cust = SR.node[u][r][j - 1];
			int cur_cust = SR.node[u][r][j];
			Second_Route_Adjust_Time(u, r, j, AT, BT, Cust, SR, cargoArriveTime);
			SR.at[u][r][cur_cust] = SR.at[u][r][pre_cust] + Second_Cust_Time[u][pre_cust][cur_cust];
			Second_Route_Cust_Update_Time(u, r, j, pre_cust, cur_cust, AT, BT, Cust, SR, Second_Cust_Time);
		}
	}
	delete feas;
}

void ExchangeOperator03(int u, int**& routeNum, First_Node* Node, Second_Cust**& Cust, Second_Solution& SR, float*** Second_Cust_Time, float** &AT, float**&BT)
{//the (1,1) inter-route exchange operator
	int route01 = rand() % routeNum[0][0] + 1;
	int r = routeNum[route01][0];
	int nodeNum = routeNum[route01][1];
	int index01 = rand() % nodeNum + 1;
	int Cust01 = SR.node[u][r][index01];
	int pre_cust01 = SR.node[u][r][index01 - 1];
	int lat_cust01 = SR.node[u][r][index01 + 1];
	float originTime1 = Second_Cust_Time[u][pre_cust01][Cust01] + Second_Cust_Time[u][Cust01][lat_cust01];
	InsertNode * routeInsert = new InsertNode[routeNum[0][0] + 1];

	for (int k = 1; k < routeNum[0][0] + 1; k++)
	{
		int r1 = routeNum[k][0];
		float cargoArriveTime = 0.0;
		Cargo_Arrive_Time(u, r, routeNum[k][1], cargoArriveTime, Node, Cust, SR);
		Second_Route_Adjust_Time(u, r1, routeNum[k][1] + 1, AT, BT, Cust, SR, cargoArriveTime);

		if (r1 != r)
		{
			float addTime = 10000.0;
			int index = 0;
			for (int i = 1; i < routeNum[k][1] + 1; i++)
			{
				int pre_cust02 = SR.node[u][r1][i - 1];
				int cur_cust = SR.node[u][r1][i];
				int lat_cust02 = SR.node[u][r1][i + 1];
				float originTime2 = Second_Cust_Time[u][pre_cust02][cur_cust] + Second_Cust_Time[u][cur_cust][lat_cust02];
				float changeTime1 = Second_Cust_Time[u][pre_cust01][cur_cust] + Second_Cust_Time[u][cur_cust][lat_cust01];
				float changeTime2 = Second_Cust_Time[u][pre_cust02][Cust01] + Second_Cust_Time[u][Cust01][lat_cust02];

				if (SR.at[u][r][pre_cust01] + Second_Cust_Time[u][pre_cust01][cur_cust] - AT[r][0] <= Cust[u][cur_cust].Latest_ST
					&& SR.at[u][r][pre_cust01] + Second_Cust_Time[u][pre_cust01][cur_cust] + BT[r][0] >= Cust[u][cur_cust].Earliest_ST
					&& SR.restC[u][r] + Cust[u][Cust01].Total_Demand - Cust[u][cur_cust].Total_Demand >= 0
					&& SR.restC[u][r1] + Cust[u][cur_cust].Total_Demand - Cust[u][Cust01].Total_Demand >= 0
					&& SR.time[u][r] + originTime1 - changeTime1 <= T2
					&& SR.time[u][r1] + originTime2 - changeTime2 <= T2)
				{
					float ATr = 0;
					float BTr = 0;
					if (SR.at[u][r][pre_cust01] + Second_Cust_Time[u][pre_cust01][cur_cust] - Cust[u][cur_cust].Earliest_ST < AT[r][0])
						ATr = SR.at[u][r][pre_cust01] + Second_Cust_Time[u][pre_cust01][cur_cust] - Cust[u][cur_cust].Earliest_ST;
					if (Cust[u][cur_cust].Latest_ST - (SR.at[u][r][pre_cust01] + Second_Cust_Time[u][pre_cust01][cur_cust]) < BT[r][0])
						BTr = Cust[u][cur_cust].Latest_ST - (SR.at[u][r][pre_cust01] + Second_Cust_Time[u][pre_cust01][cur_cust]);

					if ((lat_cust01 == 0)
						|| ((lat_cust01 > 0) && (SR.at[u][r][pre_cust01] + changeTime1 - AT[r][0] <= Cust[u][lat_cust01].Latest_ST) && (SR.at[u][r][pre_cust01] + changeTime1 + BT[r][0] >= Cust[u][lat_cust01].Earliest_ST)))
					{
						if (SR.at[u][r1][pre_cust02] + Second_Cust_Time[u][pre_cust02][Cust01] - AT[r1][0] <= Cust[u][Cust01].Latest_ST
							&& SR.at[u][r1][pre_cust02] + Second_Cust_Time[u][pre_cust02][Cust01] + BT[r1][0] >= Cust[u][Cust01].Earliest_ST)
						{
							if ((lat_cust02 == 0)
								|| ((lat_cust02 > 0) && (SR.at[u][r1][pre_cust02] + changeTime2 - AT[r1][0] <= Cust[u][Cust01].Latest_ST) && (SR.at[u][r1][pre_cust02] + changeTime2 + BT[r1][0] >= Cust[u][Cust01].Earliest_ST)))
							{
								float ATr1 = 0;
								float BTr1 = 0;
								if (SR.at[u][r1][pre_cust02] + Second_Cust_Time[u][pre_cust02][Cust01] - Cust[u][Cust01].Earliest_ST < AT[r1][0])
									ATr1 = SR.at[u][r1][pre_cust02] + Second_Cust_Time[u][pre_cust02][Cust01] - Cust[u][Cust01].Earliest_ST;
								if (Cust[u][Cust01].Latest_ST - (SR.at[u][r1][pre_cust02] + Second_Cust_Time[u][pre_cust02][Cust01]) < BT[r1][0])
									BTr1 = Cust[u][Cust01].Latest_ST - (SR.at[u][r1][pre_cust02] + Second_Cust_Time[u][pre_cust02][Cust01]);

								float adjust_r = AT[r][0] - ATr;
								float adjust_r1 = AT[r1][0] - ATr1;
								if (addTime > changeTime1 - originTime1 + changeTime2 - originTime2 - adjust_r - adjust_r1)
								{
									addTime = changeTime1 - originTime1 + changeTime2 - originTime2 - adjust_r - adjust_r1;
									index = i;
								}
							}
						}
					}
				}
			}
			if (index != 0)
			{
				routeInsert[k].car = k;
				routeInsert[k].pos = index;
				routeInsert[k].addtime = addTime;
			}
		}
	}

	int finalRoute = 1;
	for (int r1 = 1; r1 < routeNum[0][0] + 1; r1++)
	{
		if (routeInsert[finalRoute].addtime > routeInsert[r1].addtime)
		{
			finalRoute = r1;
		}
	}

	int index02 = routeInsert[finalRoute].pos;
	if (index02 > 0)
	{
		int k = routeNum[routeInsert[finalRoute].car][0];
		int Cust02 = SR.node[u][k][index02];
		SR.node[u][k][index02] = Cust01;
		SR.restC[u][k] = SR.restC[u][k] - Cust[u][Cust02].Total_Demand + Cust[u][Cust01].Total_Demand;
		float cargoArriveTime02 = 0.0;
		Cargo_Arrive_Time(u, k, routeNum[routeInsert[finalRoute].car][1], cargoArriveTime02, Node, Cust, SR);
		for (int i = index02; i < routeNum[routeInsert[finalRoute].car][1] + 1; i++)
		{
			int pre_cust = SR.node[u][k][i - 1];
			int cur_cust = SR.node[u][k][i];
			Second_Route_Adjust_Time(u, k, i, AT, BT, Cust, SR, cargoArriveTime02);
			SR.at[u][k][cur_cust] = SR.at[u][k][pre_cust] + Second_Cust_Time[u][pre_cust][cur_cust];
			Second_Route_Cust_Update_Time(u, k, i, pre_cust, cur_cust, AT, BT, Cust, SR, Second_Cust_Time);
		}
		int fir_cust02 = SR.node[u][k][1];
		SR.dt[u][k] = SR.at[u][k][fir_cust02] - Second_Cust_Time[u][0][fir_cust02];
		int las_cust02 = SR.node[u][k][routeNum[routeInsert[finalRoute].car][1]];
		SR.time[u][k] = SR.at[u][k][las_cust02] + Second_Cust_Time[u][las_cust02][0];

		SR.node[u][r][index01] = Cust02;
		SR.restC[u][r] = SR.restC[u][r] - Cust[u][Cust01].Total_Demand + Cust[u][Cust02].Total_Demand;
		float cargoArriveTime01 = 0.0;
		Cargo_Arrive_Time(u, r, nodeNum, cargoArriveTime01, Node, Cust, SR);
		for (int i = index01; i < nodeNum + 1; i++)
		{
			int pre_cust = SR.node[u][r][i - 1];
			int cur_cust = SR.node[u][r][i];
			Second_Route_Adjust_Time(u, r, i, AT, BT, Cust, SR, cargoArriveTime01);
			SR.at[u][r][cur_cust] = SR.at[u][r][pre_cust] + Second_Cust_Time[u][pre_cust][cur_cust];
			Second_Route_Cust_Update_Time(u, r, i, pre_cust, cur_cust, AT, BT, Cust, SR, Second_Cust_Time);
		}
		int fir_cust01 = SR.node[u][r][1];
		SR.dt[u][r] = SR.at[u][r][fir_cust01] - Second_Cust_Time[u][0][fir_cust01];
		int las_cust01 = SR.node[u][r][nodeNum];
		SR.time[u][r] = SR.at[u][r][las_cust01] + Second_Cust_Time[u][las_cust01][0];
	}
}

void SecondEchelonDisturb(int u, First_Node* Node, Second_Cust**& Cust, Second_Solution& SR, float*** Second_Cust_Time, float***& Second_Cust_Cost)
{//The total function of the second-echelon disturbance
	int **routeNum = new int*[SV_Num + 1];
	for (int r = 0; r < SV_Num + 1; r++)
	{
		routeNum[r] = new int[2];
		for (int j = 0; j < 2; j++)
		{
			routeNum[r][j] = 0;
		}
	}
	for (int r = 1; r < SV_Num + 1; r++)
	{
		for (int i = 1; i < C_Num + 1; i++)
		{
			if (SR.node[u][r][i] > 0 && SR.node[u][r][i] <= C_Num)
			{
				if (i == 1)
				{
					routeNum[0][0] += 1;
					routeNum[routeNum[0][0]][0] = r;
				}
				routeNum[routeNum[0][0]][1] += 1;
			}
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
	Second_Solution newSR;
	newSR = SR;

	ExchangeOperator01(u, routeNum, Node, SR, Cust, Second_Cust_Time, Second_Cust_Cost, AT, BT, newSR);

	for (int r = 0; r < SV_Num + 1; r++)
	{
		for (int i = 0; i < 2; i++)
		{
			routeNum[r][i] = 0;
		}
	}
	for (int r = 1; r < SV_Num + 1; r++)
	{
		for (int i = 1; i < C_Num + 1; i++)
		{
			if (SR.node[u][r][i] > 0 && SR.node[u][r][i] <= C_Num)
			{
				if (i == 1)
				{
					routeNum[0][0] += 1;
					routeNum[routeNum[0][0]][0] = r;
				}
				routeNum[routeNum[0][0]][1] += 1;
			}
		}
	}
	newSR = SR;

	ExchangeOperator02(u, routeNum, Node, Cust, SR, Second_Cust_Time, newSR, AT, BT);

	for (int r = 0; r < SV_Num + 1; r++)
	{
		for (int i = 0; i < 2; i++)
		{
			routeNum[r][i] = 0;
		}
	}
	for (int r = 1; r < SV_Num + 1; r++)
	{
		for (int i = 1; i < C_Num + 1; i++)
		{
			if (SR.node[u][r][i] > 0 && SR.node[u][r][i] <= C_Num)
			{
				if (i == 1)
				{
					routeNum[0][0] += 1;
					routeNum[routeNum[0][0]][0] = r;
				}
				routeNum[routeNum[0][0]][1] += 1;
			}
		}
	}

	ExchangeOperator03(u, routeNum, Node, Cust, SR, Second_Cust_Time, AT, BT);

	for (int r = 0; r < SV_Num + 1; r++)
	{
		for (int i = 0; i < 2; i++)
		{
			routeNum[r][i] = 0;
		}
	}
	for (int r = 1; r < SV_Num + 1; r++)
	{
		for (int i = 1; i < C_Num + 1; i++)
		{
			if (SR.node[u][r][i] > 0 && SR.node[u][r][i] <= C_Num)
			{
				if (i == 1)
				{
					routeNum[0][0] += 1;
					routeNum[routeNum[0][0]][0] = r;
				}
				routeNum[routeNum[0][0]][1] += 1;
			}
		}
	}
}