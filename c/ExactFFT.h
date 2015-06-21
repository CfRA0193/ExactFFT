/*----------------------------------------------------------------------+
 |  filename:   ExactFFT.h                                              |
 |----------------------------------------------------------------------|
 |  version:    7.10                                                    |
 |  revision:   07/09/2014  11:41                                       |
 |  author:     �������� ���� ��������� (DrAF)                         |
 |  e-mail:     draf@mail.ru                                            |
 |  purpose:    ����������� FFT                                         |
 |----------------------------------------------------------------------*/

 #ifndef _exactfft_h
 #define _exactfft_h

 #include <stdlib.h>
 #include <math.h>

 //------------------------------------
 //- ������� & DUMP
 //------------------------------------
 // !���������������� ��� ������ �����
 #define DUMP_MODE
 #define DUMP_NAME "VS_C.dump"
 //------------------------------------

#ifdef DUMP_MODE

 #include <stdio.h>
 #include <direct.h>

#endif

 //------------------------
 //- ���� ����������������
 //------------------------

 // ���������
 //                              3.14159265358979323846264338328
 #define M_PI                    3.14159265358979323846 //..264338328
 #define M_2PI                   2 * M_PI
 #define FLOAT_MIN               3.4E-38 // ���������� ������� ��� �������� float
 #define MAX_FFT_DIFF            1E-7    // ������������ ����������� FFT
 #define MIN_FRAME_WIDTH         8       // ���������� "�������" ������ ���� FFT
 #define MAX_KAISER_BETA         28      // Max. beta (Kaiser window), SL: ~240 dB
 #define MAX_PATH                256     // ������������ ����� ����

 // "���������" ���������
 #define TRUE                    1       // "������"
 #define FALSE                   0       // "����"
 #define DIRECT                  1       // ����������� ������� ������� FFT
 #define REVERSE                 0       // ����������� ��������� ������� FFT
 #define USING_NORM              1       // ������������ ������������
 #define NOT_USING_NORM          0       // �� ������������ ������������
 #define USING_TAPER_WINDOW      1       // ������������ ������������ ����
 #define NOT_USING_TAPER_WINDOW  0       // �� ������������ ������������ ����
 #define USING_POLYPHASE         1       // ������������ ���������� FFT
 #define NOT_USING_POLYPHASE     0       // �� ������������ ���������� FFT

 // ���������� ������������� �������� ���������
 #define SAFE_DELETE(ptr)  if (ptr != NULL) \
                           { \
                               free(ptr); \
                               ptr = NULL; \
                           }
 
 // min / max
 #define min(a, b) ((a) < (b) ? (a) : (b))
 #define max(a, b) ((a) > (b) ? (a) : (b))

 // ������ "���������" ��� ������
 typedef int bool;

 //---------------------------------------------
 //- ���� ������������ ���� FFT
 //---------------------------------------------
 // �������������� ����: PS - "Peak Sidelobe" (��������� ������� ��������, ��)
 enum CosTW {
                NONE,
                RECTANGULAR_13dbPS,
                HANN_31dbPS,
                HAMMING_43dbPS,
                MAX_ROLLOFF_3_TERM_46dbPS,
                BLACKMAN_58dbPS,
                COMPROMISE_3_TERM_64dbPS,
                EXACT_BLACKMAN_68dbPS,
                MIN_SIDELOBE_3_TERM_71dbPS,
                MAX_ROLLOFF_4_TERM_60dbPS,
                COMPROMISE1_4_TERM_82dbPS,
                COMPROMISE2_4_TERM_93dbPS,
                BLACKMAN_HARRIS_92dbPS,
                NUTTALL_93dbPS,
                BLACKMAN_NUTTALL_98dbPS,
                ROSENFIELD
 };

 //-------------------------
 //- ��������� "������ FFT"
 //-------------------------
 typedef struct
 {
     //-------------------------------------------------------------------------
     int     N;       // ���������� ����� FFT
     int     NN;      // ���-�� ����� (re + im) FFT
     int     NPoly;   // ������������� ��� ����������� FFT ���������� �����
     int     NNPoly;  // ���-�� ����� (re + im) ����������� FFT
     int     CosTW;   // ��� ����������� ������������� ���� (���� ��� - ������������ ���� �������)
     double  Beta;    // ����������� ����������� "beta" ���� �������     
     int     PolyDiv; // �������� "������������" FFT ("0" - ������� FFT)
     //-------------------------------------------------------------------------
     int    *FFT_P;   // ������ ��������� ������� ���������� ������ ����� FFT
     int    *FFT_PP;  // ������ ��������� �������... (��� ����������� FFT)
     double *FFT_TW;  // ������������ ����
     //-------------------------------------------------------------------------

 } CFFT_Object;

 //---------------------------------------------
 //- ��������� "��������� ��������������� CFFT"
 //---------------------------------------------
 typedef struct
 {
     //-------------------------------------------------------------------------
     bool AllOK; // ��������� ������������ ��������
     //-------------------------------------------------------------------------
     double MaxDiff_ACH; // ������������ ������� �� ������� �������� ���
     double MaxDiff_ALG_to_EXP_to_ALG; // Max. ������� ALG -> EXP � �������
     double MaxDiff_FORWARD_BACKWARD;  // Max. ������� FORVARD + BACKWARD
     double MaxDiff_FORWARD_BACKWARD_AntiTW; //...�� �� + �����. ����� TW
     double MaxDiff_PhaseLR;   // ����. ������� �� ������� �������� ���� ���
     double CFFT_Process_time; // ����� ������ CFFT_Process()
     double CFFT_Explore_time; // ����� ������ CFFT_Explore()
     //-------------------------------------------------------------------------

 } CFFT_SelfTestResult;

#endif