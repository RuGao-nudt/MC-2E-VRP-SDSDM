using namespace std;

//Calling different instances requires changes
const int D_Num = 2;    //all CDCs (city distribution centers)
const int S_Num = 3;   //all satellites
const int C_Num = 10;  //customers(per satellite)
const int H_Num = 2;  //commodities
const int BV_Num = 8; //all urban trucks
const int SV_Num = 15;//electric vehicles(per satellite)
const int XH = 2;    //the first/second/third/fourth test instance of this special scale

//parameters
const int Q1 = 200;          //the capacity of urban truck
const int Q2 = 50;           //the capacity of electric vehicle
const float BV_Speed = 40.0;//the speed of urban truck
const float SV_Speed = 15.0;//the speed of electric vehicle
const float BV_coe = 0.6;   //unit distance cost of urban truck
const float SV_coe = 0.1;   //unit distance cost of electric vehicle
const int T1 = 8;          //the maximum working time of urban truck
const int T2 = 6;          //the maximum working time of electric vehicle
const int F1 = 50;          //the fixed cost of urban truck
const int F2 = 20;          //the fixed cost of electric vehicle
const float tope = 1;       //The necessary transfer time in two echelon
const int Pel = 10;        //unit waiting cost
const long L = 1000;       //a large number

//Variable parameters
const long sateIter = 300;//value={300,400}
const long custIter = 300;//value={300},sateIter¡ÁcustIter=The total disturbance number of a satellite
const long continuIter = 200000;//value={200000¡¢300000},The successive iteration number of the best solution
const double alpha = 0.5;//value={0.3,0.5}£¬The selected probability of the first-echelon removal operators
const double cooling = 0.95;//value={0.9£¬0.95}£¬The cooling coefficient in Simulate annealing algorithm

//CDCs/satellites information
class First_Node
{
public:
	float Supply[H_Num + 1];	
	float Demand[H_Num + 1];	
	float X;     
	float Y;     
	int orig[H_Num + 1][D_Num + 1]; 
	float at_limit[H_Num + 1]; 
	float at[H_Num + 1]; 
public:
	First_Node()
	{
		for (int m = 0; m < H_Num + 1; m++)
		{
			Demand[m] = 0.0;	
			Supply[m] = 0.0;
			at_limit[m] = 24;
			at[m] = 24;
			for (int u = 0; u < D_Num + 1; u++)
			{
				orig[m][u] = 0;
			}
		}
		X = 0;  
		Y = 0;  

	}
};

//customers information
class Second_Cust  
{
public:
	float X; 
	float Y; 
	float Earliest_ST; 
	float Latest_ST;
	float Demand[H_Num + 1];
	float Total_Demand;
public:
	Second_Cust()
	{
		X = 0.0;
		Y = 0.0;
		Earliest_ST = 0.0;
		Latest_ST = 0.0;
		for (int m = 0; m < H_Num + 1; m++)
		{
			Demand[m] = 0.0;	
		}
		Total_Demand = 0.0;
	}
};

//the information of first-echelon routes
class First_Solution
{
public:
	int node[S_Num + 2]; 
	int satenum; 
	int dc; 
	int supply[S_Num + 2][H_Num + 1];  
	float restC;
	float time;
	float dt; 
	float at[S_Num + 2];
	float delay[S_Num + 2]; 
public:
	First_Solution()
	{
		dc = 0;
		satenum = 0;
		restC = Q1;
		time = 0.0;
		dt = 0.0;
		for (int u = 0; u < S_Num + 2; u++)
		{
			node[u] = 0;
			at[u] = 0.0;
			delay[u] = 8;
			for (int m = 0; m < H_Num + 1; m++)
			{
				supply[u][m] = 0.0;
			}
		}
			
	}
};

//the information of second-echelon routes
class Second_Solution
{
public:
	int node[S_Num + 1][SV_Num + 1][C_Num + 1]; 
	float dt[S_Num + 1][SV_Num + 1];
	float at[S_Num + 1][SV_Num + 1][C_Num + 1]; 
	float time[S_Num + 1][SV_Num + 1];
	float restC[S_Num + 1][SV_Num + 1];
	float Cust_Demand[S_Num + 1][C_Num + 1];
	int load[S_Num + 1][SV_Num + 1][H_Num + 1]; 
public:
	Second_Solution()
	{
		for (int u = 0; u < S_Num + 1; u++)
		{
			for (int r = 0; r < SV_Num + 1; r++)
			{
				for (int i = 0; i < C_Num + 1; i++)
				{
					node[u][r][i] = 0;
					dt[u][r] = 8.0;
					at[u][r][i] = 0.0;
					time[u][r] = 0.0;
					restC[u][r] = Q2;
					Cust_Demand[u][i] = 0;
				}
				for (int m = 0; m < H_Num + 1; m++)
				{
					load[u][r][m] = 0;
				}
			}
		}
	}
};

 //Insertion point
class InsertNode 
{
public:
	int car;
	int pos; 
	float addtime; 
	int feas; 
public:
	InsertNode()
	{
		car = 0;
		pos = -1;
		addtime = 10000.0;
		feas = 1;
	}
};

//customer set
class Cust_Set
{
public:
	int CargoType[H_Num + 1];
	int cust[C_Num + 1];
	float time; 
public:
	Cust_Set()
	{
		for (int m = 0; m < H_Num + 1; m++)
		{
			CargoType[m] = 0;
		}
		for (int i = 0; i < C_Num + 1; i++)
		{
			cust[i] = 0;
		}
		time = 0.0;
	}
};

//objective value
class Total_Cost
{
public:
	float First_Vehicle_Cost;
	float First_Route_Cost[BV_Num + 1];
	float First_Total_Route_Cost;

	float Second_Vehicle_Cost[S_Num + 1];
	float Second_Total_Vehicle_Cost;
	float Second_ES_Route_Cost[S_Num + 1];            
	float Second_Route_Cost[S_Num + 1][SV_Num + 1];
	float Second_Total_Route_Cost;

	float link_cost;
	float TotalCost;
public:
	Total_Cost()
	{
		First_Vehicle_Cost = 0;
		First_Total_Route_Cost = 0;

		Second_Total_Route_Cost = 0;
		Second_Total_Vehicle_Cost = 0;

		link_cost = 0;
		TotalCost = 0;
		for (int k = 0; k < BV_Num + 1; k++)
		{
			First_Route_Cost[k] = 0;
		}
		for (int u = 0; u < S_Num + 1; u++)
		{
			Second_Vehicle_Cost[u] = 0;
			Second_ES_Route_Cost[u] = 0;
			for (int r = 0; r < SV_Num + 1; r++)
			{
				Second_Route_Cost[u][r] = 0;
			}
		}

	}

};

//The supply-demand relationship of a certain satellite
class Sate_Supply_Demand
{
   public:
	   int car[BV_Num + 1];
	   int supply[BV_Num + 1];
	   int index[BV_Num + 1];
	   int cargoTpye[BV_Num + 1][H_Num + 1];
	   int num;
public:
	Sate_Supply_Demand()
	{
		num = 0;
		for (int k = 0; k < BV_Num + 1; k++)
		{
			car[k] = 0;
			supply[k] = 0;
			index[k] = 0;
			for (int m = 0; m < H_Num + 1; m++)
			{
				cargoTpye[k][m] = 0;
			}
		}
	}
};

//functions
void Sate_Arrive_Time_Limit(int S_Num, int C_Num, int H_Num, First_Node*& Node, Second_Cust**& Cust, float*** Second_Cust_Time);
void Cargo_Orig(int D_Num, int S_Num, int H_Num, int**& cargo_orig, First_Node*& Node);
void Calculate_Cargo_Arrive_Time(int S_Num, int H_Num, int BV_Num, First_Node*& Node, First_Solution*& FR);
void Second_Route_Adjust_Time(int u, int r, int i, float**& AT, float**& BT, Second_Cust**& Cust, Second_Solution& SR, float cargoArriveTime);
void Second_Route_Cust_Update_Time(int u, int r, int i, int pre_cust, int cur_cust, float**& AT, float**& BT, Second_Cust**& Cust, Second_Solution& SR, float*** Second_Cust_Time);
void Initial_First__Route(int D_Num, int S_Num, int H_Num, int BV_Num, First_Node*& Node, First_Solution*& FR, float** First_Distance, float** First_Nodes_Time, int**& cargo_orig);
void Initial_Second_Route(int S_Num, int C_Num, int H_Num, int SV_Num, First_Node* Node, First_Solution*& FR, float**& First_Nodes_Time, Second_Cust**& Cust, Second_Solution& SR, float*** Second_Cust_Time);
void FirstEchelonDisturb(int u, int D_Num, int S_Num, int C_Num, int H_Num, int BV_Num, First_Node* Node, First_Solution*& FR, float** First_Nodes_Time, float** First_Nodes_Cost, int &removeOperator, double *&removePro, float *&removeScore, Second_Cust**& Cust, float*** Second_Cust_Time);
void SecondEchelonDisturb(int u, First_Node* Node, Second_Cust**& Cust, Second_Solution& SR, float*** Second_Cust_Time, float***& Second_Cust_Cost);