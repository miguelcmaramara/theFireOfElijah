#include <Arduino.h>
#include <Pixy2.h>

// This is the main Pixy object 
Pixy2 pixy;

/**
 * Editable variables
 **/
unsigned const int Y_POLY_DEGREE = 2;
unsigned const int XNZ_POLY_DEGREE = 1; 
unsigned const int PROJ_SIGNATURE = 2; 

// time variables
// double, used to input to equation
double t;
unsigned long t0;
unsigned long deltaT;

// tracking variables;
unsigned int count = 0;
int i; 
int j; 
int k;

double **x;
double **y;
double **z;
double xPara[XNZ_POLY_DEGREE+1];
double yPara[Y_POLY_DEGREE+1];
double zPara[XNZ_POLY_DEGREE+1];

void createXYZArr(){
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


void printArr(double arr[],int size){
  Serial.print("[");
  for(int i = 0; i < size; i++){
    Serial.print(arr[i]);
    if(i < size - 1) Serial.print(", ");
  }
  Serial.print("]\n");
}

void timeArray(double arr[], unsigned int size, unsigned long time, int coord){
  // this function fills all but the last of the array with time time
  arr[0] = 1;
  for(int i = 1; i < size - 1; i++)
    arr[i] = (double)time/1000;
  arr[size] = (double)coord;
}

void curveFit(double **arr, double res[], unsigned int resSize){
  int i;
  int ii;
  int j;
  /** the input has t as t, not as t**n
   * 
   *  I'll assume that the dimensions are [r][c] [ressize][resSize + 1]
   * {{1 t**2 t**3 t**4 t**5 y**0},
   *  {1 t**2 t**3 t**4 t**5 y**1},
   *  {1 t**2 t**3 t**4 t**5 y**2},
   *  {1 t**2 t**3 t**4 t**5 y**3},
   *  {1 t**2 t**3 t**4 t**5 y**5}}
   **/
  // convert time into time^deg
  for(i = 0; i < resSize; i++){
    for(j = 1; j < resSize; j++ )
      arr[i][j] = pow(arr[i][j], j);
  }
  
  //solve lin eq
  // does each row backwards as to perserve the first value
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
}


void setup() {
  Serial.begin(9600);
  Serial.print("Starting...\n");
  //pixy.init();

  createXYZArr();


  // fill values
  k = 0;
  for (i = 0; i < Y_POLY_DEGREE + 1; i++){
    for (j = 0; j < Y_POLY_DEGREE + 2; j++){
      y[i][j] = ++k;
      Serial.print(y[i][j]);
      Serial.print(" ");
    }
    printArr(y[i],Y_POLY_DEGREE + 2);
  }

}

void loop() { 
  /*******
   * I'm assuming that there is only
   * one object on the screeen at the same time
   *******/

  // reset counters
  count = 0;
  t0 = millis();

  curveFit(y,yPara, Y_POLY_DEGREE + 1);
  // Serial.print(xPara[0]);
  // Serial.print(" ");
  // Serial.print(xPara[1]);
  // Serial.print(" ");
  printArr(yPara,Y_POLY_DEGREE + 1);
  Serial.print(t0);
  Serial.print("\n");
  delay(10000);


  // do{
  //   pixy.ccc.getBlocks();
  //   for(i = 0; i < pixy.ccc.numBlocks; i++){
  //     if(pixy.ccc.blocks[i].m_signature == PROJ_SIGNATURE){
  //       t = (t0 - millis()) / 1000;

  //       if(count < XNZ_POLY_DEGREE + 2)
  //         timeArray(x[count], XNZ_POLY_DEGREE + 1, t, pixy.ccc.blocks[i].m_x);
  //       timeArray(y[count], Y_POLY_DEGREE + 1, t, pixy.ccc.blocks[i].m_y);
  //       count++;
  //     }
  //   }
  // } while(pixy.ccc.numBlocks && count <= Y_POLY_DEGREE + 2);


  // if(count > 0){
  //   deltaT = millis() - t0;
  //   Serial.print(count);
  //   Serial.print(" readings in ");
  //   Serial.print((double)deltaT / (double)1000);
  //   Serial.print(" seconds\n");
  //   Serial.print((double)count / (double)deltaT * (double)1000);
  //   Serial.print(" readings per second\n");
  // }

}