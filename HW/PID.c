#include<stdio.h>

float Kd = 0.1;
float Ki = 0.1;
float pre_error = 0;
float integral_sum = 0;

float D_Control(float error, float dt)
{
    float derivative = (error - pre_error) / dt;    // �̺а� ���ϱ�
    float d_control = Kd * derivative;              // �̺��̵� ����
    pre_error = error;                              // ���� ������Ʈ

    return d_control;
}

float I_Control(float error, float dt)
{
    integral_sum += (error * dt);               // �������� ����
    float i_control = Ki * integral_sum;           // �̺��̵� ����

    return i_control;
}
