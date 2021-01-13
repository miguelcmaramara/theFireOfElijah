#include <Arduino.h>
#include <Pixy2.h>

// This is the main Pixy object 
Pixy2 pixy;

/**
 * Editable variables
 **/
unsigned const int Y_POLY_DEGREE = 2;
unsigned const int XNZ_POLY_DEGREE = 2; 
unsigned const int PROJ_SIGNATURE = 2; 
unsigned const int SKIP_PARAM = 3; 
unsigned const int NUM_INPUT = 4; 

// time variables
double t;
unsigned long t0;
unsigned long deltaT;

// tracking variables;
unsigned int count = 0;
unsigned int trueCount = 0;
int i; 
int j; 
int k;

double **x;
double **y;
double **z;
double xPara[XNZ_POLY_DEGREE+1];
double yPara[Y_POLY_DEGREE+1];
double zPara[XNZ_POLY_DEGREE+1];
int X[100];
int Y[100];
double T[100];

void OLD_createXYZArr(){
  // Only purpose is to create arrays
  x = new double *[XNZ_POLY_DEGREE + 1];
  for(i = 0; i < XNZ_POLY_DEGREE + 1; i++)
    x[i] = new double [XNZ_POLY_DEGREE + 2];

  y = new double *[Y_POLY_DEGREE + 1];
  for(i = 0; i < Y_POLY_DEGREE + 1; i++)
    y[i] = new double [Y_POLY_DEGREE + 2];

  z = new double *[XNZ_POLY_DEGREE + 1];
  for(i = 0; i < XNZ_POLY_DEGREE + 1; i++)
    z[i] = new double [XNZ_POLY_DEGREE + 2];
}
void createXYZArr(){
  // Only purpose is to create arrays
  x = new double *[NUM_INPUT];
  for(i = 0; i < NUM_INPUT; i++)
    x[i] = new double [XNZ_POLY_DEGREE + 2];

  y = new double *[NUM_INPUT];
  for(i = 0; i < NUM_INPUT; i++)
    y[i] = new double [Y_POLY_DEGREE + 2];

  z = new double *[NUM_INPUT];
  for(i = 0; i < NUM_INPUT; i++)
    z[i] = new double [XNZ_POLY_DEGREE + 2];
}

//Logistical functions
void printArr(unsigned long arr[],int size){
  Serial.print("[");
  for(int i = 0; i < size; i++){
    Serial.print(arr[i]);
    if(i < size - 1) Serial.print(", ");
  }
  Serial.print("]\n");
}
void printArr(double arr[],int size){
  Serial.print("[");
  for(int i = 0; i < size; i++){
    Serial.print(arr[i]);
    if(i < size - 1) Serial.print(", ");
  }
  Serial.print("]\n");
}
void printArr(int arr[],int size){
  Serial.print("[");
  for(int i = 0; i < size; i++){
    Serial.print(arr[i]);
    if(i < size - 1) Serial.print(", ");
  }
  Serial.print("]\n");
}
void fillTestValues(){
  // fill values
  k = 0;
  for (i = 0; i < NUM_INPUT; i++){
    for (j = 0; j < Y_POLY_DEGREE + 2; j++){
      y[i][j] = ++k;
      x[i][j] = ++k;
      if(k%5 == 0 && k%3 == 1) k = 5;
      Serial.print(y[i][j]);
      Serial.print(" ");
    }
    printArr(y[i],Y_POLY_DEGREE + 2);
  }
}

/***
 * Matrix Math
 * */
void multiply(double **A, double **B, int m, int p, int n, double **C){
  /***
   * AB = C
   * m = rows of A
   * p = col of A / rows of B
   * n = cols = B
   ***/
  int r;
  int c;
  int i;

  for(r = 0; r < m; r++)
    for(c = 0; c < n; c++){
      C[r][c] = 0;
      for(i = 0; i < p; i++)
        C[r][c] += A[r][i] * B[i][c];
    }
}
void multiply(double **A, double B[], int m, int p, double C[]){
  /***
   * Overloaded multiplication function for multiplying by a vector
   * A*B = C
   * m = rows of A
   * p = col of A / rows of B
   * n = 1 = columns of B
   * C => mx1
   ***/
  int r;
  int c;
  int i;

  for(r = 0; r < m; r++){
      C[r] = 0;
      for(i = 0; i < p; i++)
        C[r]+= A[r][i] * B[i];
    }
}
void invert(double **A, int m){
  /***
   * A^-1 = B
   * A = mxm matrix
   * B = Identity mxm matrix
   **/
  int i;
  int j;
  int ii;


  double **B;
  B = new double *[m];
  for(i = 0; i < m; i++)
    B[i] = new double[m];


  for(i = 0; i < m; i++)
    for(j = 0; j < m; j++)
      if(i == j) B[i][j] = 1;
      else B[i][j] = 0;
      

  for(i = 0; i < m; i++)
    for(ii = 0; ii < m; ii++){
      if(i == ii) continue;
      // Identity Array
      for(j = m - 1; j >= 0; j--)
        B[ii][j] -= B[i][j] * A[ii][i] / A[i][i];

      // Input Array
      for(j = m - 1; j >= i; j--)
        A[ii][j] -= A[i][j] * A[ii][i] / A[i][i];
    }


  // divide each by the non1 row of m
  for(i = 0; i < m; i++)
    for(j = 0; j < m; j++)
      B[i][j] /= A[i][i];

  for(i = 0; i < m; i++)
    for(j = 0; j < m; j++)
      A[i][j] = B[i][j];
}
void transpose(double **A, int m, int n, double **T){
  /***
   * A = mxn matrix
   * T = nxm matrix
   ***/
  int i;
  int j;

  for(i = 0; i < m; i++)
    for(j = 0; j < n; j++)
      T[j][i] = A[i][j];
}

void timeArray(double arr[], unsigned int size, double time, int coord){
  // this function fills all but the last of the array with time time
  arr[0] = 1;
  for(int i = 1; i < size; i++)
    arr[i] = (double)time;
  arr[size - 1] = (double)coord;
}

void adjustTime(double **arr, int m,int n){
  int i;
  int j;
  // convert time into time^deg
  for(i = 0; i < m; i++){
    for(j = 1; j < n - 1; j++ )
      arr[i][j] = pow(arr[i][j], j);
  }
}
void fitLSRL(double **arr, double res[], int m, int n){
  // xhat = inv(A^t * A) * At *b
  int i;
  int ii;
  int j;
  //solve lin eq
  // does each row backwards as to perserve the first value
  /*
  for(i = 0; i < resSize; i++)
    for(ii = 0; ii < resSize; ii++){
      if(i == ii) continue;
      for(j = resSize; j >= i; j--){
        arr[ii][j] -= arr[i][j] * arr[ii][i] / arr[i][i];
      }
    }

  // divide by only value left
  for(i = 0; i < resSize; i++){
    res[i] = arr[i][resSize] / arr[i][i];
  }
    */
}

void skipCount(){
  do{
    pixy.ccc.getBlocks();
    for(i = 0; i < pixy.ccc.numBlocks; i++){
      if(pixy.ccc.blocks[i].m_signature == PROJ_SIGNATURE){
        t = ((double)millis() - (double)t0)/1000;
        Serial.print("Time: ");
        Serial.print(t);
        Serial.print("\n");
        if(trueCount < XNZ_POLY_DEGREE + 1 && count%SKIP_PARAM == 0){
          timeArray(x[trueCount], XNZ_POLY_DEGREE + 2, t, pixy.ccc.blocks[i].m_x);
          printArr(x[trueCount],XNZ_POLY_DEGREE + 2);
        }
        if(trueCount < Y_POLY_DEGREE + 1 && count%SKIP_PARAM == 0){
          timeArray(y[trueCount], Y_POLY_DEGREE + 2, t, pixy.ccc.blocks[i].m_y);
          printArr(y[trueCount],Y_POLY_DEGREE + 2);
          trueCount++;
        }
        X[count] = pixy.ccc.blocks[i].m_x;
        Y[count] = pixy.ccc.blocks[i].m_y;
        T[count] = t;
        count++;
      }
    }
  } while(pixy.ccc.numBlocks);
}

void setup() {
  Serial.begin(9600);
  Serial.print("Starting...\n");
  // pixy.init();
  createXYZArr();

  fillTestValues();



}

void loop() { 
  /*******
   * I'm assuming that there is only
   * one object on the screeen at the same time
   *******/

  // reset counters
  count = 0;
  trueCount = 0;
  t0 = millis();
  Serial.print(t0);

  // skipCount();

  // testing Matrix Math
  Serial.print("\n");
  Serial.print("Beginning Test");
  Serial.print("\n");
  // Printing Arrays
  for(i = 0; i < NUM_INPUT; i++)
    printArr(y[i], Y_POLY_DEGREE+2);
  Serial.print("\n");
  for(i = 0; i < NUM_INPUT; i++)
    printArr(x[i], XNZ_POLY_DEGREE+2);
  
  //Benchmarking process
  Serial.print("\n");
  Serial.print(millis());
  Serial.print("\n");
  invert(y,4);
Serial.print(millis());
  Serial.print("\n");

  //Printing result
  for(i = 0; i < NUM_INPUT; i++)
    printArr(y[i], Y_POLY_DEGREE+2);
  Serial.print("\n");
  Serial.print(millis() - t0);
  Serial.print("\n");
  delay(100000);

  // for inputing into the curve - to plug into matlab
  if(count > 0){
    deltaT = millis() - t0;
    Serial.print(count);
    Serial.print(" readings in ");
    Serial.print((double)deltaT / (double)1000);
    Serial.print(" seconds\n");
    Serial.print((double)count / (double)deltaT * (double)1000);
    Serial.print(" readings per second\n");

    Serial.print("xPara =");
    // curveFit(x,xPara, XNZ_POLY_DEGREE + 1);
    printArr(xPara,XNZ_POLY_DEGREE + 1);
    Serial.print("yPara =");
    // curveFit(y,yPara, Y_POLY_DEGREE + 1);
    printArr(yPara,Y_POLY_DEGREE + 1);
    Serial.print("X= ");
    printArr(X,count);
    Serial.print("Y= ");
    printArr(Y,count);
    Serial.print("T= ");
    printArr(T,count);
    Serial.print("tCount = ");
    Serial.print(T[count - 1]);
    Serial.print("\n\n\n");




    Serial.print("Start: ");
    Serial.print(t0);
    Serial.print("\n");
    Serial.print("End: ");
    Serial.print(millis());
    Serial.print("\n");

  }

  // for(i = 0; i < Y_POLY_DEGREE + 1; i++)
  //   printArr(y[i],Y_POLY_DEGREE + 2);


}