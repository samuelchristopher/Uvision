#include "MKL25Z4.h"                    // Device header



#define PTD0_PIN 0
#define PTD1_PIN 1
#define PTD2_PIN 2
#define PTD3_PIN 3
#define PRESCALAR 7
#define PS_ACTUAL 128
#define DEFAULT_PWM_FREQ 50
#define TRUE 1
#define FALSE 0

typedef enum Direction
{
	LEFT, RIGHT, FORWARD, BACKWARD, FORWARD_LEFT, FORWARD_RIGHT, BACKWARD_LEFT, BACKWARD_RIGHT
};


//pinD0 channel 0, D1 channel 1, D2 channel 2, D3 channel 3
/* intiPWM() */

void initPWM(void)
{
	SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK;
	
	PORTD->PCR[PTD0_PIN] &= ~PORT_PCR_MUX_MASK;
	PORTD->PCR[PTD0_PIN] |= PORT_PCR_MUX(4);
	
	PORTD->PCR[PTD1_PIN] &= ~PORT_PCR_MUX_MASK;
	PORTD->PCR[PTD1_PIN] |= PORT_PCR_MUX(4);
	
	PORTD->PCR[PTD2_PIN] &= ~PORT_PCR_MUX_MASK;
	PORTD->PCR[PTD2_PIN] |= PORT_PCR_MUX(4);
	
	PORTD->PCR[PTD3_PIN] &= ~PORT_PCR_MUX_MASK;
	PORTD->PCR[PTD3_PIN] |= PORT_PCR_MUX(4);
	
	SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK;
	
	SIM->SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);
	
	TPM0->MOD = (DEFAULT_SYSTEM_CLOCK/PS_ACTUAL) / DEFAULT_PWM_FREQ;
	
	//Timer  settings
	TPM0->SC &= ~((TPM_SC_CMOD_MASK) | (TPM_SC_PS_MASK));
	TPM0->SC |= (TPM_SC_CMOD(1) | TPM_SC_PS(PRESCALAR));
	TPM0->SC &= ~(TPM_SC_CPWMS_MASK);
	
	//timer channel settings for pwm mode activation
	TPM0_C0SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK) |(TPM_CnSC_MSB_MASK) |(TPM_CnSC_MSA_MASK));
	TPM0_C0SC |= (TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1));
	TPM0_C1SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK) |(TPM_CnSC_MSB_MASK) |(TPM_CnSC_MSA_MASK));
	TPM0_C1SC |= (TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1));
	TPM0_C2SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK) |(TPM_CnSC_MSB_MASK) |(TPM_CnSC_MSA_MASK));
	TPM0_C2SC |= (TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1));
	TPM0_C3SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK) |(TPM_CnSC_MSB_MASK) |(TPM_CnSC_MSA_MASK));
	TPM0_C3SC |= (TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1));
	
}

void setDutyCycle(float percent, int channel)
{
	float multiple = percent/100.0;
	
	switch(channel) {
	case 0:
		TPM0_C0V = (unsigned int)(TPM0->MOD * multiple);
	  break;
	case 1:
		TPM0_C1V = (unsigned int)(TPM0->MOD * multiple);
	  break;
	case 2:
		TPM0_C2V = (unsigned int)(TPM0->MOD * multiple);
	  break;
	case 3:
		TPM0_C3V = (unsigned int)(TPM0->MOD * multiple);
	  break;
	}
	
}

void setFreq(unsigned int value, int channel) 
{
	
	float step_size;
	unsigned int steps_needed;
	step_size = ((float)PS_ACTUAL/(float)DEFAULT_SYSTEM_CLOCK);
	steps_needed = (unsigned int)((1/(float)value)/step_size);
	
	TPM0->MOD = steps_needed;
	
	//after you set the new frequency, the COV value needs to change
	setDutyCycle(50.0, channel);
}

/*
	1 nof = 1/48Mhz = 21 nanoseconds
*/
static void delay(volatile uint32_t nof) {
  while(nof!=0) {
    __asm("NOP");
    nof--;
  }
}

void leftMotorControl(float power, int direction) {
	if(direction) {
		setDutyCycle(power, PTD0_PIN);
		setDutyCycle(0, PTD1_PIN);
		
	} else {
		setDutyCycle(0, PTD0_PIN);
		setDutyCycle(power, PTD1_PIN);
	}
}

void rightmotorControl(float power, int direction) {
	if(direction) {
		setDutyCycle(power, PTD2_PIN);
		setDutyCycle(0, PTD3_PIN);
		
	} else {
		setDutyCycle(0, PTD2_PIN);
		setDutyCycle(power, PTD3_PIN);
	}
}

void stop() {
	leftMotorControl(0, 0);
	rightMotorControl(0, 0);
}

void move(Direction direction) {
	switch(direction) {
	case LEFT:
		leftMotorControl(50, 1); // make left move backwards
		rightMotorControl(50, 0); // make right move forwards
		break;
	case RIGHT:
		leftMotorControl(50, 0); // make left move forwards
		rightMotorControl(50, 1); // make right move backwards
		break;
	case FORWARD:
		leftMotorControl(50, 0); // make left move forwards
		rightMotorControl(50, 0); // make right move forwards
		break;
	case BACKWARD:
		leftMotorControl(50, 1); // make left move backwards
		rightMotorControl(50, 1); // make right move backwards
		break;
	case FORWARD_LEFT:
		leftMotorControl(50, 0); // make left move forwards
		rightMotorControl(75, 0); // make right move forwards slightly faster
		break;
	case FORWARD_RIGHT:
		leftMotorControl(75, 0); // make left move forwards slightly faster
		rightMotorControl(50, 0); // make right move forwards 
		break;
	case BACKWARD_RIGHT:
		leftMotorControl(75, 1); // make left move backwards slightly faster
		rightMotorControl(50, 1); // make right move backwards 
		break;
	case BACKWARD_LEFT:
		leftMotorControl(50, 1); // make left move backwards 
		rightMotorControl(75, 1); // make right move backwards slightly faster
		break;
	default:
		stop();
		break;
	}

	delay(0xFFFF);
	stop();
}

void UARTMotor(uint8_t info;)
{
	uint8_t info;

	switch (info)
	{ 
	case 1:
		move(FORWARD);
		break;
	case 2:
		move(BACKWARD);
		break;
	case 3:
		move(LEFT);
		break;
	case 4:
		move(RIGHT);
		break;
	case 6:
		move(FORWARD_LEFT);
		break;
	case 7:
		move(FORWARD_RIGHT);
		break;
	case 8:
		move(BACKWARD_LEFT);
		break;
	case 9:
		move(BACKWARD_RIGHT);
		break;
	default:
		break;
	}
}

int main()
{
	initPWM();
	setDutyCycle(0, 0);
	setDutyCycle(0, 1);
	setDutyCycle(0, 2);
	setDutyCycle(0, 3);
	
	while(1){
		
		rightmotorControl(50,0);
		delay(0xFFFFF);
		delay(0xFFFFF);
		rightmotorControl(50,1);
		delay(0xFFFFF);
		delay(0xFFFFF);
		
	}	
}
