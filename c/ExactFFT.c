/*----------------------------------------------------------------------+
 |  filename:   ExactFFT.c                                              |
 |----------------------------------------------------------------------|
 |  version:    7.10                                                    |
 |  revision:   07/09/2014  11:41                                       |
 |  author:     �������� ���� ��������� (DrAF)                         |
 |  e-mail:     draf@mail.ru                                            |
 |  purpose:    ����������� FFT                                         |
 |----------------------------------------------------------------------*/

 #ifndef _exactfft_c
 #define _exactfft_c

 #include "ExactFFT.h"
 #include "Windows.h"

 extern enum CosTW;

 /// <summary>
 /// ��������� ��������� �� �������� �����
 /// <summary>
 /// <param name="dirName"> ��� ����������. </param>
 /// <param name="fileName"> ��� �����. </param>
 /// <param name="toWrite"> ������� ���� ��� ������? </param>
 FILE * GetStreamPointer(char *dirName, char *fileName, bool toWrite)
 {
     // ������ ���� � ����� �����
     char path[MAX_PATH];

     // �������������� ������ ����
     if(dirName != NULL)
     {
        strcpy(path, dirName);
        strcat(path, "\\");
        strcat(path, fileName);

     } else
     {
        strcpy(path, fileName);
     }

     // ������ � ������
     return fopen(path, toWrite ? "wb" : "rb");
 }

 /// <summary>
 /// ����� ������� int-�� � ���� � �������� �����
 /// <summary>
 /// <param name="arr"> �������� ������. </param>
 /// <param name="N"> ���������� ��������� ��� ������. </param>
 /// <param name="dirName"> ��� ���������� �����. </param>
 /// <param name="fileName"> ��� ����� �����. </param>
 void DumpInt(int *arr, int N, char *dirName, char *fileName)
 {
     int i;
     int data;
     FILE *f = GetStreamPointer(dirName, fileName, TRUE);

     for(i = 0; i < N; ++i)
     {
         data = arr[i];
         fwrite(&data, sizeof(int), 1, f);
     }

     fflush(f); fclose(f);
 }

 /// <summary>
 /// ����� ������� double-�� � ���� � �������� �����
 /// <summary>
 /// <param name="arr"> �������� ������. </param>
 /// <param name="N"> ���������� ��������� ��� ������. </param>
 /// <param name="dirName"> ��� ���������� �����. </param>
 /// <param name="fileName"> ��� ����� �����. </param>
 void DumpDouble(double *arr, int N, char *dirName, char *fileName)
 {
     int i;
     double data;
     FILE *f = GetStreamPointer(dirName, fileName, TRUE);

     for(i = 0; i < N; ++i)
     {
         data = arr[i];
         fwrite(&data, sizeof(double), 1, f);
     }

     fflush(f); fclose(f);
 }

 /// <summary>
 /// �������� �� ������������� ���������
 /// </summary>
 /// <param name="arg"> �������� ���������. </param>
 /// <param name="logBase"> ��������� ���������. </param>
 double LogX(double arg, double logBase)
 {
     return log(arg) / log(logBase);
 }

 /// <summary>
 /// ���������� �������� � ��������� ����� ������� ������
 /// </summary>
 /// <param name="arg"> ������� ��������. </param>
 int ToLowerPowerOf2(int arg)
 {
     return (int)pow(2, (int)(LogX(arg, 2)));
 }

 /// <summary>
 /// ��������� ������� ��������� ���� FFT
 /// </summary>
 /// <param name="FFT_Node"> ����� ���������. </param>
 /// <param name="sampFreq"> ������� �������������. </param>
 /// <param name="isComplex"> ����������� �����? </param>
 /// <param name="fftObj"> ������ FFT, ��� �������� ���������� �������. </param>
 double FreqNode(double FFT_Node, double sampFreq, bool isComplex,
                 CFFT_Object *fftObj)
 {
     return (FFT_Node * sampFreq) / ((double)fftObj->N * (isComplex ? 2.0 : 1.0));
 }

 /// <summary>
 /// ��������� ���� FFT �� �������� �������
 /// </summary>
 /// <param name="freqNode"> �������� �������. </param>
 /// <param name="sampFreq"> ������� �������������. </param>
 /// <param name="isComplex"> ����������� �����? </param>
 /// <param name="fftObj"> ������ FFT, ��� �������� ���������� �������. </param>
 double FFT_Node(double freqNode, double sampFreq, bool isComplex,
                 CFFT_Object *fftObj)
 {
     return (freqNode * ((double)fftObj->N * (isComplex ? 2.0 : 1.0))) / sampFreq;
 }

 /// <summary>
 /// ������������ ����
 /// </summary>
 /// <param name="phase"> �������� ���� ��� ������������. </param>
 double PhaseNorm(double phase)
 {
     if(phase > 0) while (phase >= M_PI)  phase -= M_2PI;
              else while (phase <= -M_PI) phase += M_2PI;
 
     return phase;
 }

 /// <summary>
 /// ���������� � ������ �������� ���������� ���������� �����������
 /// </summary>
 /// <param name="im"> ������ ����� ������������ �����. </param>
 /// <param name="re"> �������������� ����� ������������ �����. </param>
 double Safe_atan2(double im, double re)
 {
     return (((re < 0) ? -re : re) < FLOAT_MIN) ? 0 : atan2(im, re);
 }

 /// <summary>
 /// ���������� ������� ��������� ������� ���������� ������ ����� FFT
 /// </summary>
 /// <param name="fftObj"> ������ FFT, ��� �������� ���������� �������. </param>
 void fill_FFT_P(CFFT_Object *fftObj)
 {
     int i, j, shift;

     // �������� ������ ��� ������ ������������ FFT...
     SAFE_DELETE(fftObj->FFT_P);
     fftObj->FFT_P = (int *)calloc(fftObj->NN, sizeof(int));

     // ������������� ������� ������������...
     memset(fftObj->FFT_P, 0x00, (fftObj->NN * sizeof(int)));

     // ��������� ������ ��������� ������� ���������� ������...
     for(j = 0; j < LogX(fftObj->N, 2); ++j)
     {
        for(i = 0; i < fftObj->N; ++i)
        {
            fftObj->FFT_P[i << 1] = ((fftObj->FFT_P[i << 1] << 1) +
                                    ((i >> j) & 1));
        }
     }

     shift = (fftObj->FFT_P[2] == (fftObj->N >> 1)) ? 1 : 0;
     for(i = 0; i < fftObj->NN; i += 2)
     {
         fftObj->FFT_P[i + 1] = (fftObj->FFT_P[i + 0] <<= shift) + 1;
     }
 }

 /// <summary>
 /// ���������� ������� ��������� ������� ���������� ������ ����� FFT
 /// (��� ����������� FFT)
 /// </summary>
 /// <param name="fftObj"> ������ FFT, ��� �������� ���������� �������. </param>
 void fill_FFT_PP(CFFT_Object *fftObj)
 {
     int i, j;
     
     // �������� ������ ��� ������ ������������ FFT...
     SAFE_DELETE(fftObj->FFT_PP);
     fftObj->FFT_PP = (int *)calloc(fftObj->NNPoly, sizeof(int));

     // ������������� ������� ������������...
     memset(fftObj->FFT_PP, 0x00, (fftObj->NNPoly * sizeof(int)));

     // ��������� ������ ��������� ������� ���������� ������
     // (��� ����������� FFT)...
     for(j = 0; j < LogX(fftObj->NPoly, 2); ++j)
     {
        for(i = 0; i < fftObj->NPoly; ++i)
        {
            fftObj->FFT_PP[i << 1] = ((fftObj->FFT_PP[i << 1] << 1) +
                                     ((i >> j) & 1));
        }
     }

     for(i = 0; i < fftObj->NNPoly; i += 2)
     {
         fftObj->FFT_PP[i + 1] = (fftObj->FFT_PP[i + 0] <<= 1) + 1;
     }
 }

 /// <summary>
 /// ���������� ������� ����������� ������������� ����
 /// </summary>
 /// <param name="fftObj"> ������ FFT, ��� �������� ���������� �������. </param>
 void fill_FFT_TW_Cosine(CFFT_Object *fftObj)
 {     
     int i;

     // ����������� ��������� ������������� ����������� ����
     double arg, wval, a0, a1, a2, a3, ad;
     
     // �������� ������ ��� ������������ ����...
     fftObj->FFT_TW = (double *)calloc(fftObj->NN, sizeof(double));

     // �������������� ����: PS - "Peak Sidelobe" (��������� ������� ��������, ��)
     switch (fftObj->CosTW)
     {
        case RECTANGULAR_13dbPS:         { a0 = 1.0;       a1 = 0;         a2 = 0;         a3 = 0;         ad = 1.0;     break; }
        case HANN_31dbPS:                { a0 = 1.0;       a1 = 1.0;       a2 = 0;         a3 = 0;         ad = 2;       break; }
        case HAMMING_43dbPS:             { a0 = 0.54;      a1 = 0.46;      a2 = 0;         a3 = 0;         ad = 1.0;     break; }
        case MAX_ROLLOFF_3_TERM_46dbPS:  { a0 = 0.375;     a1 = 0.5;       a2 = 0.125;     a3 = 0;         ad = 1.0;     break; }
        case BLACKMAN_58dbPS:            { a0 = 0.42;      a1 = 0.5;       a2 = 0.08;      a3 = 0;         ad = 1.0;     break; }
        case COMPROMISE_3_TERM_64dbPS:   { a0 = 0.40897;   a1 = 0.5;       a2 = 0.09103;   a3 = 0;         ad = 1.0;     break; }
        case EXACT_BLACKMAN_68dbPS:      { a0 = 7938.0;    a1 = 9240.0;    a2 = 1430.0;    a3 = 0;         ad = 18608.0; break; }
        case MIN_SIDELOBE_3_TERM_71dbPS: { a0 = 0.4243801; a1 = 0.4973406; a2 = 0.0782793; a3 = 0;         ad = 1.0;     break; }
        case MAX_ROLLOFF_4_TERM_60dbPS:  { a0 = 10.0;      a1 = 15.0;      a2 = 6.0;       a3 = 1;         ad = 32.0;    break; }
        case COMPROMISE1_4_TERM_82dbPS:  { a0 = 0.338946;  a1 = 0.481973;  a2 = 0.161054;  a3 = 0.018027;  ad = 1.0;     break; }
        case COMPROMISE2_4_TERM_93dbPS:  { a0 = 0.355768;  a1 = 0.487396;  a2 = 0.144232;  a3 = 0.012604;  ad = 1.0;     break; }
        default:
        case BLACKMAN_HARRIS_92dbPS:     { a0 = 0.35875;   a1 = 0.48829;   a2 = 0.14128;   a3 = 0.01168;   ad = 1.0;     break; }
        case NUTTALL_93dbPS:             { a0 = 0.355768;  a1 = 0.487396;  a2 = 0.144232;  a3 = 0.012604;  ad = 1.0;     break; }
        case BLACKMAN_NUTTALL_98dbPS:    { a0 = 0.3635819; a1 = 0.4891775; a2 = 0.1365995; a3 = 0.0106411; ad = 1.0;     break; }
        case ROSENFIELD:                 { a0 = 0.762;     a1 = 1.0;       a2 = 0.238;     a3 = 0;         ad = a0;      break; }
     }

     // ��������� ������������ ���� ��������������...
     for(i = 0; i < fftObj->N; ++i)
     {
         arg  = (2.0 * M_PI * i) / (double)fftObj->N;
         wval = (a0 - a1 * cos(arg) + a2 * cos(2 * arg) - a3 * cos(3 * arg)) / ad;
         fftObj->FFT_TW[(i << 1) + 1] = fftObj->FFT_TW[(i << 1) + 0] = wval;
     }
 }

 /// <summary>
 /// ���������. ������� ������� �������� ������� ������� ����
 /// </summary>
 /// <param name="arg"> �������� �������. </param>
 /// <returns> �������� �������. </returns>
 double BesselI0(double arg)
 {
     double numerator, denominator, z, z1, z2, z3, z4, z5,
            z6, z7, z8, z9, z10, z11, z12, z13, z_1, z_2;

     if(arg == 0.0)
     {
         return 1.0;
     }
     
     z = arg * arg;

     z1  = z * 0.210580722890567e-22 + 0.380715242345326e-19;
     z2  = z * z1  + 0.479440257548300e-16;
     z3  = z * z2  + 0.435125971262668e-13;
     z4  = z * z3  + 0.300931127112960e-10;
     z5  = z * z4  + 0.160224679395361e-7;
     z6  = z * z5  + 0.654858370096785e-5;
     z7  = z * z6  + 0.202591084143397e-2;
     z8  = z * z7  + 0.463076284721000e0;
     z9  = z * z8  + 0.754337328948189e2;
     z10 = z * z9  + 0.830792541809429e4;
     z11 = z * z10 + 0.571661130563785e6;
     z12 = z * z11 + 0.216415572361227e8;
     z13 = z * z12 + 0.356644482244025e9;

     numerator = z * z13 + 0.144048298227235e10;

     z_1 = z - 0.307646912682801e4;
     z_2 = z * z_1 + 0.347626332405882e7;
     denominator = z * z_2 - 0.144048298227235e10;

     return -numerator / denominator;
 }

 /// <summary>
 /// ����� ���������� ������������� ���� (���� �������)
 /// </summary>
 /// <param name="fftObj"> ������ FFT, ��� �������� ���������� �������. </param>
 void fill_FFT_TW_Kaiser(CFFT_Object *fftObj)
 {
     int i, j;
     double norm, arg, w;

     // �������� ������ ��� ������ ������������ FFT...
     SAFE_DELETE(fftObj->FFT_TW);
     fftObj->FFT_TW = (double *)calloc(fftObj->NN, sizeof(double));

     // ����������� ����������� ���� �������
     norm = BesselI0(fftObj->Beta);

     // ��������� ������������ ����...
     for(i = 1; i <= (fftObj->N >> 1); ++i)
     {
         // arg = Beta * sqrt(1-(((2*(i-1))/(N-1))-1)^2);
         arg = fftObj->Beta *
                              sqrt(
                                    1 - pow(
                                             (
                                                (double)((i - 1) << 1)
                                              /
                                                (double)(fftObj->N - 1)
                                             ) - 1
                                        , 2)
                                   );

         w = BesselI0(arg) / norm;

         j = i - 1; // �������� ������ �� ���� "1" � ���� "0"
         fftObj->FFT_TW[(j << 1) + 0] = w; // left re
         fftObj->FFT_TW[(j << 1) + 1] = w; // left im
         fftObj->FFT_TW[(fftObj->NN - 2) - (j << 1) + 0] = w; // right re
         fftObj->FFT_TW[(fftObj->NN - 2) - (j << 1) + 1] = w; // right im
     }
 }

 /// <summary>
 /// "���������" ������� FFT
 /// </summary>
 /// <param name="fftObj"> ������ FFT, ��� �������� ���������� �������. </param>
 /// <returns> ��������� ���� ������������ ������������ FFT. </returns>
 int CFFT_Inspector(CFFT_Object *fftObj)
 {
     // ������ ���� �� ����� ���� ������ ���������-�����������!
     if((fftObj->N     < MIN_FRAME_WIDTH) ||
        (fftObj->NPoly < MIN_FRAME_WIDTH) ||
        (fftObj->Beta  > MAX_KAISER_BETA))
     {
         return FALSE;
     }

     return TRUE;
 }

 /// <summary>
 /// "����������" ������� FFT
 /// </summary>
 /// <param name="fftObj"> ������ FFT, ��� �������� ���������� �������. </param>
 void CFFT_Destructor(CFFT_Object *fftObj)
 {
     SAFE_DELETE(fftObj->FFT_P);
     SAFE_DELETE(fftObj->FFT_PP);
     SAFE_DELETE(fftObj->FFT_TW);
     SAFE_DELETE(fftObj);
 }

 /// <summary>
 /// ������� �������� FFT
 /// </summary>
 /// <param name="frameWidth"> ������ �����. </param>
 /// <param name="cosTW"> ��� ����������� ������������� ���� (���� ��� - ������������ ���� �������). </param>
 /// <param name="beta"> ����������� ����������� ���� �������. </param>
 /// <param name="polyDiv2"> ������� ������������ ��� ������� ������. </param>
 CFFT_Object * CFFT_Init(int frameWidth, int cosTW, double beta, int polyDiv2)
 {
     // ������-���������
     CFFT_Object *fftObj = (CFFT_Object *)calloc(1, sizeof(CFFT_Object));

     // ���������� ����� �������
     fftObj->N = ToLowerPowerOf2(frameWidth);  // ������ ����� FFT
     fftObj->NN = fftObj->N << 1;              // ���-�� ����� (re + im)
     fftObj->NPoly = fftObj->N >> polyDiv2;    // ������ ����������� ����� FFT
     fftObj->CosTW = cosTW;                    // ��� ����������� ������������� ����
     fftObj->NNPoly = fftObj->NPoly << 1;      // ���-�� ����� ����������� FFT
     fftObj->Beta = beta;                      // ����-�� �����. ���� �������
     fftObj->PolyDiv = 1 << polyDiv2;          // ���������� ��������

     fill_FFT_P(fftObj);  // ������ ��������� ������� ����. ������ ����� FFT
     fill_FFT_PP(fftObj); // ������ ��������� �������... (��� ����������� FFT)
     
     if(fftObj->CosTW == NONE) //...���� �� ������ ������������ ���� ����������� ����
     {
         fill_FFT_TW_Kaiser(fftObj); // ������������ ���� �������

     } else
     {
         fill_FFT_TW_Cosine(fftObj); // ���������� ������������ ����
     }

     // ������������ �������� �� ������� �����...

#ifdef DUMP_MODE

     mkdir(DUMP_NAME);

     DumpInt(fftObj->FFT_P,     fftObj->NN,     DUMP_NAME, "FFT_P.int32");
     DumpInt(fftObj->FFT_PP,    fftObj->NNPoly, DUMP_NAME, "FFT_PP.int32");
     DumpDouble(fftObj->FFT_TW, fftObj->NN,     DUMP_NAME, "FFT_TW.double");

#endif

     // ���� ��������� ��������� �� ������������� �����...
     if(!CFFT_Inspector(fftObj))
     {
         //...- ������� ������.
         CFFT_Destructor(fftObj);
     }

     // ���������� ������ "FFT"
     return fftObj;
 }

 /// <summary>
 /// ������� �������� FFT
 /// </summary>
 /// <param name="frameWidth"> ������ �����. </param>
 /// <param name="cosTW"> ��� ����������� ������������� ����. </param>
 /// <param name="polyDiv2"> ������� ������������ ��� ������� ������. </param>
 CFFT_Object * CFFT_Constructor_Cosine(int frameWidth, int cosTW, int polyDiv2)
 {
     // ���������� ������ "FFT"
     return CFFT_Init(frameWidth, cosTW, MAX_KAISER_BETA, polyDiv2);
 }

 /// <summary>
 /// ������� �������� FFT
 /// </summary>
 /// <param name="frameWidth"> ������ �����. </param>
 /// <param name="beta"> ����������� ����������� ���� �������. </param>
 /// <param name="polyDiv2"> ������� ������������ ��� ������� ������. </param>
 CFFT_Object * CFFT_Constructor_Kaiser(int frameWidth, double beta, int polyDiv2)
 {
     // ���������� ������ "FFT"
     return CFFT_Init(frameWidth, NONE, beta, polyDiv2);
 }

 /// <summary>
 /// �������� ����� ������������ FFT
 /// </summary>
 /// <param name="FFT_S"> ������ ������� ������
 /// ("�����" � "������" ������ - ���./�����.). </param>
 /// <param name="FFT_S_Offset"> �������� ������ ��� ������� ��
 /// ������� ������� FFT_S. </param>
 /// <param name="FFT_T"> �������� ������ �������������. </param>
 /// <param name="useTaperWindow"> ������������ ������������ ����? </param>
 /// <param name="recoverAfterTaperWindow"> �������������� ��������
 /// ������������� ���� �� �������� �������? </param>
 /// <param name="useNorm"> ������������ ������������ 1/N? </param>
 /// <param name="direction"> ����������� �������������� (TRUE - ������).
 /// </param>
 /// <param name="usePolyphase"> ������������ ���������� FFT? </param>
 /// <param name="fftObj"> ������ FFT, ��� �������� ���������� �������. </param>
 void CFFT_Process(double *FFT_S, int FFT_S_Offset, double *FFT_T,
                   bool useTaperWindow, bool recoverAfterTaperWindow,
                   bool useNorm, bool direction, bool usePolyphase,
                   CFFT_Object *fftObj)
 {
     int i, j, mmax, isteps, n, istep, ii, m, jj;
     double isign, theta, sin05Th, wpr, wpi, wr, wi, tempr, tempi, wtemp;
     
     // ������������� ������������� ���� ���������
     // ������ ��� ������ ��������������
     if(direction && useTaperWindow)
     {
         if(!usePolyphase)
         {
             // ������� FFT
             // ������ �����, ����� ��� �������
             for(i = 0; i < fftObj->NN; ++i)
             {
                 FFT_T[i] = fftObj->FFT_TW[fftObj->FFT_P[i]] *
                            FFT_S[fftObj->FFT_P[i] + FFT_S_Offset];
             }
         }
         else
         {
             // ���������� FFT
             // ������������ ���������� ������ �� ������ �������
             for(i = 0; i < fftObj->NNPoly; ++i)
             {
                 FFT_T[i] = 0;

                 // ����������� ����� ������� ����� (� ������������
                 // � ����������� ���������)
                 for(j = 0; j < fftObj->PolyDiv; ++j)
                 {
                     FFT_T[i] += fftObj->FFT_TW[fftObj->FFT_PP[i] +
                                 (j * fftObj->NNPoly)] * FFT_S[fftObj->FFT_PP[i] +
                                 (j * fftObj->NNPoly) + FFT_S_Offset];
                 }
             }
         }
     }
     else
     {
         // �������� ������ ��� ������...
         // �� ��� ������������� ����
         for(i = 0; i < fftObj->NN; ++i)
         {
             FFT_T[i] = FFT_S[fftObj->FFT_P[i] + FFT_S_Offset];
         }
     }

     // ������������ ������������� ������������ ��� � ������ � ������ ��
     // ������ ������� ��������� (��� ���� �������� 100% �����������)
     if((!direction) && (!useNorm))
     {
         for(i = 0; i < fftObj->NNPoly; ++i)
         {
             FFT_T[i] /= fftObj->N;
         }
     }

     // FFT Routine
     isign  = direction ? -1 : 1;
     mmax   = 2;
     isteps = 1;
     n = usePolyphase ? fftObj->NNPoly : fftObj->NN;
     while(n > mmax)
     {
         isteps++;
         istep   = mmax << 1;
         theta   = isign * ((2 * M_PI) / mmax);
         sin05Th = sin(0.5 * theta);
         wpr = -(2.0 * (sin05Th * sin05Th));
         wpi = sin(theta);
         wr  = 1.0;
         wi  = 0.0;

         for(ii = 1; ii <= (mmax >> 1); ++ii)
         {
             m = (ii << 1) - 1;
             for(jj = 0; jj <= ((n - m) >> isteps); ++jj)
             {
                 i = m + (jj << isteps);
                 j = i + mmax;
                 tempr = wr * FFT_T[j - 1] - wi * FFT_T[j];
                 tempi = wi * FFT_T[j - 1] + wr * FFT_T[j];
                 FFT_T[j - 1]  = FFT_T[i - 1] - tempr;
                 FFT_T[j - 0]  = FFT_T[i - 0] - tempi;
                 FFT_T[i - 1] += tempr;
                 FFT_T[i - 0] += tempi;
             }
             wtemp = wr;
             wr = wr * wpr - wi    * wpi + wr;
             wi = wi * wpr + wtemp * wpi + wi;
         }
         mmax = istep;
     }

     // ������������ ������������� ������������ ��� � ������ � ������
     // �� ������ ������� ��������� (��� ���� �������� 100% �����������)
     if(direction && useNorm)
     {
         for(i = 0; i < n; ++i)
         {
             FFT_T[i] /= fftObj->N;
         }
     }

     // ������������ ������������ ���� (���� ��� ����� ���� � ��������� ������
     // - ��������� �������������� ���������� � �������� ��� ������ ����)
     if((!direction) && useTaperWindow && recoverAfterTaperWindow)
     {
         for(i = 0; i < fftObj->NN; ++i)
         {
             FFT_T[i] = ((fftObj->FFT_TW[i] == 0) ?
                        0 : (FFT_T[i] / fftObj->FFT_TW[i]));
         }
     }
 }

 /// <summary>
 /// ������������ "������" � "�������" �������: ("�����" -
 /// �������������� ����� �������� ������, "������" - ������ �����)
 /// </summary>
 /// <param name="FFT_T"> �������� ������ �������������. </param>
 /// <param name="MagL"> ��������� "������" ������. </param>
 /// <param name="MagR"> ��������� "�������" ������. </param>
 /// <param name="ACH"> ��� (��������� ��������� "�������" ������ � ���������
 /// "������" - ��� "�����" / "����"). </param>
 /// <param name="ArgL"> �������� "������" ������. </param>
 /// <param name="ArgR"> �������� "�������" ������. </param>
 /// <param name="PhaseLR"> �������� ���� ��� ������� ("������" �����
 /// "�����"). </param>
 /// <param name="usePolyphase"> ������������ ���������� FFT? </param>
 /// <param name="fftObj"> ������ FFT, ��� �������� ���������� �������. </param>
 void CFFT_Explore(double *FFT_T, double *MagL, double *MagR, double *ACH,
                   double *ArgL, double *ArgR, double *PhaseLR, bool usePolyphase,
                   CFFT_Object *fftObj)
 {
     int N, i;
     double magL, magR, FFT_T_i_Re, FFT_T_i_Im, FFT_T_N_i_Re, FFT_T_N_i_Im,
            lx, ly, rx, ry, argL, argR;

     // ���������� ����������� FFT
     N = usePolyphase ? fftObj->NPoly : fftObj->N;

     // ��������� �������� ������� ������� ��� ��������� �����
     // ("�������" ���������)
     magL = FFT_T[0];
     magR = FFT_T[1];
          
     if(MagL    != NULL) MagL[0]    = magL;
     if(MagR    != NULL) MagR[0]    = magR;
     if(ACH     != NULL) ACH[0]     = magR / ((magL == 0) ? FLOAT_MIN : magL);
     if(ArgL    != NULL) ArgL[0]    = M_PI;
     if(ArgR    != NULL) ArgR[0]    = M_PI;
     if(PhaseLR != NULL) PhaseLR[0] = 0;

     // ������ � �������������� �������
     for(i = 1; i < (N >> 1); ++i)
     {
         FFT_T_i_Re   = FFT_T[(i << 1) + 0];
         FFT_T_i_Im   = FFT_T[(i << 1) + 1];
         FFT_T_N_i_Re = FFT_T[((N - i) << 1) + 0];
         FFT_T_N_i_Im = FFT_T[((N - i) << 1) + 1];

         lx = FFT_T_i_Re   + FFT_T_N_i_Re;
         ly = FFT_T_i_Im   - FFT_T_N_i_Im;
         rx = FFT_T_i_Im   + FFT_T_N_i_Im;
         ry = FFT_T_N_i_Re - FFT_T_i_Re;

         magL = sqrt((lx * lx) + (ly * ly)) * 0.5;
         magR = sqrt((rx * rx) + (ry * ry)) * 0.5;
         argL = Safe_atan2(ly, lx);
         argR = Safe_atan2(ry, rx);

         if(MagL    != NULL) MagL[i] = magL;
         if(MagR    != NULL) MagR[i] = magR;
         if(ACH     != NULL) ACH[i]  = magR / ((magL == 0) ? FLOAT_MIN : magL);
         if(ArgL    != NULL) ArgL[i] = argL;
         if(ArgR    != NULL) ArgR[i] = argR;
         if(PhaseLR != NULL) PhaseLR[i] = PhaseNorm(argR - argL);
     }
 }

 /// <summary>
 /// ������������ ����������� ������������ FFT (��������� CFFT �� MathCAD)
 /// </summary>
 /// <param name="FFT_T"> �������� ������ �������������. </param>
 /// <param name="Mag"> ���������. </param>
 /// <param name="Arg"> ���������. </param>
 /// <param name="usePolyphase"> ������������ ���������� FFT? </param
 /// <param name="isMirror"> ���������� ����������� �������? </param>
 /// <param name="fftObj"> ������ FFT, ��� �������� ���������� �������. </param>
 void CFFT_ComplexExplore(double *FFT_T, double *Mag, double *Arg,
                          bool usePolyphase, bool isMirror,
                          CFFT_Object *fftObj)
 {
     int N, N_2, i;
     double magL, magR, FFT_T_i_Re, FFT_T_i_Im, FFT_T_N_i_Re, FFT_T_N_i_Im,
            lx, ly, rx, ry, argL, argR;

     // ���������� ����������� FFT
     N   = usePolyphase ? fftObj->NPoly : fftObj->N;
     N_2 = N >> 1;

     // ��������� �������� ������� ������� ��� ��������� �����
     // ("�������" ���������)
     lx = FFT_T[0];
     ly = FFT_T[1];
     rx = FFT_T[N + 0];
     ry = FFT_T[N + 1];
     
     if(Mag != NULL)
     {
         magL = sqrt((lx * lx) + (ly * ly));
         magR = sqrt((rx * rx) + (ry * ry));

         if(!isMirror)
         {
            Mag[0]   = magL;
            Mag[N_2] = magR;

         } else
         {
            Mag[N_2 - 1] = magL;
            Mag[N   - 1] = magR;
         }
     }

     if(Arg != NULL)
     {
         argL = Safe_atan2(ly, lx);
         argR = Safe_atan2(ry, rx);
         
         if(!isMirror)
         {
            Arg[0]   = argL;
            Arg[N_2] = argR;

         } else
         {
            Arg[N_2 - 1] = argL;
            Arg[N   - 1] = argR;
         }
     }

     // ������ � �������������� �������
     for(i = 1; i < N_2; ++i)
     {
         FFT_T_i_Re   = FFT_T[(i << 1) + 0];
         FFT_T_i_Im   = FFT_T[(i << 1) + 1];
         FFT_T_N_i_Re = FFT_T[((N - i) << 1) + 0];
         FFT_T_N_i_Im = FFT_T[((N - i) << 1) + 1];

         lx = FFT_T_i_Re;
         ly = FFT_T_i_Im;
         rx = FFT_T_N_i_Re;
         ry = FFT_T_N_i_Im;

         if(Mag != NULL)
         {
             magL = sqrt((lx * lx) + (ly * ly));
             magR = sqrt((rx * rx) + (ry * ry));

             if(!isMirror)
             {
                Mag[i]     = magL;
                Mag[N - i] = magR;

             } else
             {
                Mag[N_2 - i - 1] = magL;
                Mag[N_2 + i - 1] = magR;
             }
         }

         if(Arg != NULL)
         {
             argL = Safe_atan2(ly, lx);
             argR = Safe_atan2(ry, rx);
             
             if(!isMirror)
             {
                Arg[i]     = argL;
                Arg[N - i] = argR;

             } else
             {
                Arg[N_2 - i - 1] = argL;
                Arg[N_2 + i - 1] = argR;
             }
         }
     }
 }

 /// <summary>
 /// ������� �������� ������� double � ����� dB
 /// </summary>
 /// <param name="data"> ������ ��� ���������. </param>
 /// <param name="zero_db_level"> ������� "��������" ������. </param>
 /// <param name="fftObj"> ������ FFT, ��� �������� ���������� �������. </param>
 void dB_Scale(double *data, double zero_db_level,
               CFFT_Object *fftObj)
 {
     int i;
     for(i = 0; i < fftObj->N >> 1; ++i)
     {
         data[i] = 10 * log10(data[i] / zero_db_level); // log
     }
 }

 /// <summary>
 /// ���������������� ���������� �������� � ����� �������-���������
 /// �������������� �� ���������������� ������
 /// </summary>
 /// <param name="FFT_S"> ������ ������� ������ ("�����" � "������" ������
 /// - ���./�����.) </param>
 /// <param name="ACH_Difference"> ����������� ������������� ������� ������
 /// �� ������ � ���� ����������� �����. </param>
 /// <param name="fftObj"> ������ FFT, ��� �������� ���������� �������. </param>
 /// <returns> ��������� "��������� ������������ ���������� ��������
 /// �������-��������� ������������ �������������� �����". </returns>
 CFFT_SelfTestResult SelfTest_S(double *FFT_S, double ACH_Difference,
                                CFFT_Object *fftObj)
 {
     double *FFT_S_backward, *FFT_T, *MagL, *MagR, *ACH, *ArgL, *ArgR, *PhaseLR;
     int N2, FFT_S_Offset, i;
     bool useTaperWindow, recoverAfterTaperWindow, useNorm, direction,
          usePolyphase;
     double maxDiff, FFT_T_i_Re, FFT_T_i_Im, FFT_T_N_i_Re, FFT_T_N_i_Im,
            lx, ly, rx, ry, lx_, ly_, rx_, ry_, currentDiff;
     LARGE_INTEGER startCounter, CFFT_Process_counter, CFFT_Explore_counter, timerFrequency;
     int N_iters = 10000;

     // C�������� "��������� ������������ ���������� ��������
     // �������-��������� ������������ �������������� �����"
     CFFT_SelfTestResult selfTestResult;

     // ������ �������� ������ - ��� ���������� �� �������� ���� FFT
     FFT_S_backward = (double *)calloc(fftObj->NN, sizeof(double));

     // ������� ������
     FFT_T = (double *)calloc(fftObj->NN, sizeof(double));

     // (���������� ����� FFT / 2) - ���������� �������� ������ � �������
     N2 = fftObj->N >> 1;

     // ������� ����������� �����-�������
     MagL    = (double *)calloc(N2, sizeof(double));
     MagR    = (double *)calloc(N2, sizeof(double));
     ACH     = (double *)calloc(N2, sizeof(double));
     ArgL    = (double *)calloc(N2, sizeof(double));
     ArgR    = (double *)calloc(N2, sizeof(double));
     PhaseLR = (double *)calloc(N2, sizeof(double));

     // �� ���������� ������������ ����, �� ��������
     // � ������������� - ����������� ������
     useTaperWindow = FALSE;
     FFT_S_Offset   = 0;
     recoverAfterTaperWindow = FALSE;
     useNorm      = TRUE;
     direction    = TRUE;
     usePolyphase = FALSE;
     CFFT_Process(FFT_S, FFT_S_Offset, FFT_T, useTaperWindow,
                  recoverAfterTaperWindow, useNorm, direction,
                  usePolyphase, fftObj);

     // ��������� ������������ ������� �������������� ���������
     // ��� ��������� ��������
     CFFT_Explore(FFT_T, MagL, MagR, ACH, ArgL, ArgR, PhaseLR,
                  usePolyphase, fftObj);

     // ��������� ������������ ������� �������� � ��� - �������� ��������
     // �������� ����������� �����
     maxDiff = 0;
     for(i = 1; i < N2; ++i)
     {
         FFT_T_i_Re   = FFT_T[(i << 1) + 0];
         FFT_T_i_Im   = FFT_T[(i << 1) + 1];
         FFT_T_N_i_Re = FFT_T[((fftObj->N - i) << 1) + 0];
         FFT_T_N_i_Im = FFT_T[((fftObj->N - i) << 1) + 1];

         lx = FFT_T_i_Re   + FFT_T_N_i_Re;
         ly = FFT_T_i_Im   - FFT_T_N_i_Im;
         rx = FFT_T_i_Im   + FFT_T_N_i_Im;
         ry = FFT_T_N_i_Re - FFT_T_i_Re;

         lx_ = 2 * MagL[i] * cos(ArgL[i]);
         ly_ = 2 * MagL[i] * sin(ArgL[i]);
         rx_ = 2 * MagR[i] * cos(ArgR[i]);
         ry_ = 2 * MagR[i] * sin(ArgR[i]);

         currentDiff = fabs(lx - lx_);
         maxDiff     = (maxDiff < currentDiff) ? currentDiff : maxDiff;

         currentDiff = fabs(ly - ly_);
         maxDiff     = (maxDiff < currentDiff) ? currentDiff : maxDiff;

         currentDiff = fabs(rx - rx_);
         maxDiff     = (maxDiff < currentDiff) ? currentDiff : maxDiff;

         currentDiff = fabs(ry - ry_);
         maxDiff     = (maxDiff < currentDiff) ? currentDiff : maxDiff;
     }

     // ��������� ������������ ������� �������� �� �������������� �����
     // � ������������� � �������
     selfTestResult.MaxDiff_ALG_to_EXP_to_ALG = maxDiff;

     // �� ���������� ������������ ����, �� ��������
     // � ������������� - ����������� ��������
     useTaperWindow = FALSE;
     FFT_S_Offset   = 0;
     recoverAfterTaperWindow = FALSE;
     useNorm      = TRUE;
     direction    = FALSE;
     usePolyphase = FALSE;
     CFFT_Process(FFT_T, FFT_S_Offset, FFT_S_backward, useTaperWindow,
                  recoverAfterTaperWindow, useNorm, direction,
                  usePolyphase, fftObj);

     maxDiff = 0;
     for(i = 0; i < fftObj->N; ++i)
     {
         currentDiff = fabs(FFT_S_backward[i] - FFT_S[i]);
         maxDiff     = (maxDiff < currentDiff) ? currentDiff : maxDiff;
     }

     // ��������� ������������ ������� ����� �������-��������� ��������������
     // (��� ������������� ����)
     selfTestResult.MaxDiff_FORWARD_BACKWARD = maxDiff;

     // ���������� ������������ ����, ��������
     // � ������������� - ����������� ������
     useTaperWindow = TRUE;
     FFT_S_Offset   = 0;
     recoverAfterTaperWindow = FALSE;
     useNorm      = TRUE;
     direction    = TRUE;
     usePolyphase = FALSE;
     CFFT_Process(FFT_S, FFT_S_Offset, FFT_T, useTaperWindow,
                  recoverAfterTaperWindow, useNorm, direction,
                  usePolyphase, fftObj);

     // ���������� ����������� ������������� ����, ��������
     // � ������������� - ����������� ��������
     useTaperWindow = TRUE;
     FFT_S_Offset   = 0;
     recoverAfterTaperWindow = TRUE;
     useNorm      = TRUE;
     direction    = FALSE;
     usePolyphase = FALSE;
     CFFT_Process(FFT_T, FFT_S_Offset, FFT_S_backward, useTaperWindow,
                  recoverAfterTaperWindow, useNorm, direction,
                  usePolyphase, fftObj);

     maxDiff = 0;
     for(i = (fftObj->NN / 2); i <= ((fftObj->NN * 3) / 4); ++i)
     {
         currentDiff = fabs(FFT_S_backward[i] - FFT_S[i]);
         maxDiff     = (maxDiff < currentDiff) ? currentDiff : maxDiff;
     }

     // ��������� ������������ ������� ����� �������-���������
     // �������������� (c ������������ ������������� ����)
     selfTestResult.MaxDiff_FORWARD_BACKWARD_AntiTW = maxDiff;

     maxDiff = 0;
     for(i = 0; i < N2; ++i)
     {
         currentDiff = fabs(ACH[i] - ACH_Difference);
         maxDiff     = (maxDiff < currentDiff) ? currentDiff : maxDiff;
     }

     // ��������� ������������ ������� �� ������� �������� ���
     selfTestResult.MaxDiff_ACH = maxDiff;

     maxDiff = 0;
     for(i = 0; i < N2; ++i)
     {
         currentDiff = fabs(PhaseLR[i]);
         maxDiff     = (maxDiff < currentDiff) ? currentDiff : maxDiff;
     }

     // ��������� ������������ ������� �� ������� �������� ���� ��� �������
     selfTestResult.MaxDiff_PhaseLR = maxDiff;

     // Performance Test
     QueryPerformanceFrequency(&timerFrequency);
     CFFT_Process_counter.QuadPart = CFFT_Explore_counter.QuadPart = 0;
               
     // �� ���������� ������������ ����, �� ��������
     // � ������������� - ����������� ������
     useTaperWindow = FALSE;
     FFT_S_Offset   = 0;
     recoverAfterTaperWindow = FALSE;
     useNorm      = FALSE;
     direction    = TRUE;
     usePolyphase = FALSE;

     // CFFT_Process_time
     startCounter.QuadPart = 0;
     QueryPerformanceCounter(&startCounter);
     for (i = 0; i < N_iters; ++i)
     {
        CFFT_Process(FFT_S, FFT_S_Offset, FFT_T, useTaperWindow,
                     recoverAfterTaperWindow, useNorm, direction,
                     usePolyphase, fftObj);
     }
     QueryPerformanceCounter(&CFFT_Process_counter);
     CFFT_Process_counter.QuadPart -= startCounter.QuadPart;
     selfTestResult.CFFT_Process_time  = (long double)CFFT_Process_counter.QuadPart / (long double)timerFrequency.QuadPart;
     selfTestResult.CFFT_Process_time /= (double)N_iters;

     // CFFT_Explore_time
     startCounter.QuadPart = 0;
     QueryPerformanceCounter(&startCounter);
     for (i = 0; i < N_iters; ++i)
     {
        CFFT_Explore(FFT_T, MagL, MagR, ACH, ArgL, ArgR, PhaseLR,
                     usePolyphase, fftObj);
     }
     QueryPerformanceCounter(&CFFT_Explore_counter);
     CFFT_Explore_counter.QuadPart -= startCounter.QuadPart;
     selfTestResult.CFFT_Explore_time  = (long double)CFFT_Explore_counter.QuadPart / (long double)timerFrequency.QuadPart;
     selfTestResult.CFFT_Explore_time /= (double)N_iters;
          
     // ������������ ������� ������������ ������
     SAFE_DELETE(FFT_S);
     SAFE_DELETE(FFT_S_backward);
     SAFE_DELETE(FFT_T);
     SAFE_DELETE(MagL);
     SAFE_DELETE(MagR);
     SAFE_DELETE(ACH);
     SAFE_DELETE(ArgL);
     SAFE_DELETE(ArgR);
     SAFE_DELETE(PhaseLR);

     // �������� �� ������������ ���������� ������������
     if(selfTestResult.MaxDiff_ACH                     <= MAX_FFT_DIFF &&
        selfTestResult.MaxDiff_ALG_to_EXP_to_ALG       <= MAX_FFT_DIFF &&
        selfTestResult.MaxDiff_FORWARD_BACKWARD        <= MAX_FFT_DIFF &&
        selfTestResult.MaxDiff_FORWARD_BACKWARD_AntiTW <= MAX_FFT_DIFF &&
        selfTestResult.MaxDiff_PhaseLR                 <= MAX_FFT_DIFF)
     {
         selfTestResult.AllOK = TRUE;

     } else
     {
         selfTestResult.AllOK = FALSE;
     }

#ifdef DUMP_MODE

     // ������� �� �������� ���������� ���������������
     DumpInt(&selfTestResult.AllOK,
             1, DUMP_NAME, "AllOK.int32");

     // ������������ ������� �� ������� �������� ���
     DumpDouble(&selfTestResult.MaxDiff_ACH,
                1, DUMP_NAME, "MaxDiff_ACH.double");

     // Max. ������� ALG -> EXP � �������
     DumpDouble(&selfTestResult.MaxDiff_ALG_to_EXP_to_ALG,
                1, DUMP_NAME, "MaxDiff_ALG_to_EXP_to_ALG.double");

     // Max. ������� FORVARD + BACKWARD
     DumpDouble(&selfTestResult.MaxDiff_FORWARD_BACKWARD,
                1, DUMP_NAME, "MaxDiff_FORWARD_BACKWARD.double");

     //...�� �� + �����. ����� TW
     DumpDouble(&selfTestResult.MaxDiff_FORWARD_BACKWARD_AntiTW,
                1, DUMP_NAME, "MaxDiff_FORWARD_BACKWARD_AntiTW.double");

     // ����. ������� �� ������� �������� ���� ���
     DumpDouble(&selfTestResult.MaxDiff_PhaseLR,
                1, DUMP_NAME, "MaxDiff_PhaseLR.double");

     // ����� ������ CFFT_Process()
     DumpDouble(&selfTestResult.CFFT_Process_time,
                1, DUMP_NAME, "CFFT_Process_time.double");
     
     // ����� ������ CFFT_Explore()
     DumpDouble(&selfTestResult.CFFT_Explore_time,
                1, DUMP_NAME, "CFFT_Explore_time.double");

#endif

     // ���������� ���������� ������������
     return selfTestResult;
 }

 /// <summary>
 /// ���������������� ���������� �������� � ����� �������-���������
 /// �������������� �� ��������� ������ ("����� ���")
 /// </summary>
 /// <param name="ACH_Difference"> ����������� ������������� ������� ������
 /// �� ������ � ���� ����������� �����. </param>
 /// <param name="fftObj"> ������ FFT, ��� �������� ���������� �������. </param>
 /// <returns> ��������� "��������� ������������ ���������� ��������
 /// �������-��������� ������������ �������������� �����". </returns>
 CFFT_SelfTestResult SelfTest_RND(double ACH_Difference,
                                  CFFT_Object *fftObj)
 {    
     double randMult, randomValue;
     int i;
     
     // ������ �������� ������ - ���������
     double *FFT_S = (double *)calloc(fftObj->NN, sizeof(double));

     // �������������� ��������� ��������� �����
     srand(time(NULL));

     // ��������� ��������� �������
     randMult = 1E07;

     // ��������� �������� ������ ���������� �������...
     for(i = 0; i < fftObj->N; ++i)
     {
         // �������� �������� � ������� ��������� �����
         //(��������� ���� ������ ����)...
         randomValue = (((double)rand() / (double)RAND_MAX) * randMult) -
                       (((double)rand() / (double)RAND_MAX) * randMult);

         // "�����" ����� � "������" ����� ����� ����������
         // � "ACH_Difference" ���
         FFT_S[(i << 1) + 0] = randomValue / ACH_Difference;
         FFT_S[(i << 1) + 1] = randomValue;
     }

     // ���������� ���������� ������������...
     return SelfTest_S(FFT_S, ACH_Difference, fftObj);
 }

 /// <summary>
 /// ����� ������� ������������� �������� � �������
 /// </summary>
 /// <param name="data"> �������� ������ ��� ������ ���������. </param>
 /// <param name="startIdx"> ������� �������������. </param>
 /// <param name="finishIdx"> ������� ������. </param>
 int GetMaxIdx(double *data, int startIdx, int finishIdx)
 {
     int i, maxValIdxL, maxValIdxR;
     double currVal, maxVal;

     // �������� � �������������� ��������������...
     maxValIdxL = maxValIdxR = startIdx;
     maxVal = data[maxValIdxL];

     for(i = startIdx + 1; i <= finishIdx; ++i)
     {
         currVal = data[i];

         // ���� ������� ������ ��������� -
         // �������� ��� ������� ������...
         if(currVal > maxVal)
         {
             maxValIdxL = maxValIdxR = i;
             maxVal = currVal;

         } else
         //...� ��� ��������� ���������
         // ���������� ������ ������ ������
         if(currVal == maxVal)
         {
             maxValIdxR = i;
         }
     }

     return (maxValIdxL + maxValIdxR) >> 1;
 }
 
 /// <summary>
 /// ����� ������� ���������� ������� �� ��������� ��������
 /// </summary>
 /// <param name="Mag">  ���������. </param>
 /// <param name="L">  ����� ���������� ������� ��� �������. </param>
 /// <param name="R">  ������ ���������� ������� ��� �������. </param>
 /// <param name="sampFreq"> ������� �������������. </param>
 /// <param name="isComplex"> ����������� �����? </param>
 /// <param name="fftObj"> ������ FFT, ��� �������� ���������� �������. </param>
 /// <returns> ������ �������, ����������� �� ��������� ��������. </returns>
 double CalcExactFreq(double *Mag, int L, int R,
                      double sampFreq, bool isComplex,
                      CFFT_Object *fftObj)
 {
     int i;
     double harmSum, exactFreqIdx;
          
     // ������ ����� ��������
     harmSum = 0;
     for(i = L; i <= R; ++i)
     {
         harmSum += Mag[i];
     }

     // ��������� ������ ������ �������
     exactFreqIdx = 0;
     for(i = L; i <= R; ++i)
     {
         exactFreqIdx += (Mag[i] / harmSum) * (double)i;
     }

     // ���������� ������ �������
     return FreqNode(exactFreqIdx, sampFreq, isComplex, fftObj);
 }

 /// <summary>
 /// ����� ������� ���������� ������� �� ��������� ��������
 /// </summary>
 /// <param name="Mag">  ���������. </param>
 /// <param name="L">  ����� ���������� ������� ��� �������. </param>
 /// <param name="R">  ������ ���������� ������� ��� �������. </param>
 /// <param name="depth"> ������� ������. </param>
 /// <param name="sampFreq"> ������� �������������. </param>
 /// <param name="isComplex"> ����������� �����? </param>
 /// <param name="fftObj"> ������ FFT, ��� �������� ���������� �������. </param>
 /// <returns> ������ �������, ����������� �� ��������� ��������. </returns>
 double ExactFreq(double *Mag, int L, int R, int depth,
                  double sampFreq, bool isComplex,
                  CFFT_Object *fftObj)
 {
     int fft_N, FFT_NodeMax, startIdx, finishIdx, startIdx2, finishIdx2,
         deltaStart, deltaFinish, deltaCorr;

     // ���� ������������ ������ � ������������� ��������
     FFT_NodeMax = GetMaxIdx(Mag, L, R);
   
     // ��������� �������������� �������
     startIdx  = L;
     finishIdx = R;

     // ���������� �������� FFT
     fft_N = fftObj->N;

     // ������������ �������
     startIdx2  = startIdx  >= 0 ? startIdx : 0;
     finishIdx2 = finishIdx <= (fft_N - 1) ? finishIdx : (fft_N - 1);

     // ������� ��������� ������������ �� ������������ ������ ��������
     deltaStart  = abs(startIdx2  - startIdx);
     deltaFinish = abs(finishIdx2 - finishIdx);
     deltaCorr   = max(deltaStart, deltaFinish);
     depth      -= deltaCorr;

     // ��������� ������������� �������
     startIdx  = FFT_NodeMax - depth;
     finishIdx = FFT_NodeMax + depth;

     // ���������� ������ �������
     return CalcExactFreq(Mag, startIdx, finishIdx, sampFreq, isComplex, fftObj);
 }

 /// <summary>
 /// ����� ������� ���������� ������� �� ��������� ��������
 /// </summary>
 /// <param name="Mag">  ���������. </param>
 /// <param name="depth"> ������� ������. </param>
 /// <param name="sampFreq"> ������� �������������. </param>
 /// <param name="isComplex"> ����������� �����? </param>
 /// <param name="fftObj"> ������ FFT, ��� �������� ���������� �������. </param>
 /// <returns> ������ �������, ����������� �� ��������� ��������. </returns>
 double ExactFreqAuto(double *Mag, int depth,
                      double sampFreq, bool isComplex,
                      CFFT_Object *fftObj)
 {
     return ExactFreq(Mag, 1, (fftObj->N >> 1) - 1, depth, sampFreq, isComplex, fftObj);
 }

#endif