#include<stdio.h>

float Kd = 0.1;
float Ki = 0.1;
float pre_error = 0;
float integral_sum = 0;

float D_Control(float error, float dt)
{
    float derivative = (error - pre_error) / dt;    // 미분값 구하기
    float d_control = Kd * derivative;              // 미분이득 적용
    pre_error = error;                              // 오차 업데이트

    return d_control;
}

float I_Control(float error, float dt)
{
    integral_sum += (error * dt);               // 누적오차 적분
    float i_control = Ki * integral_sum;           // 미분이득 적용

    return i_control;
}
