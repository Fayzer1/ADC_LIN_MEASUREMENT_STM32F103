# ADC_LIN_MEASUREMENT_STM32F103
Устройство, измеряющее текущее положение резистивного датчика линейного перемещения. Сопротивлению 0 Ом соответствует 0 см, сопротивлению 3 кОм – 25 см. Результат в сантиметрах (с точностью до 1 мм) выводит на семисегментный индикатор.

### Алгоритм работы

<img width="547" height="1454" alt="image" src="https://github.com/user-attachments/assets/75c106c5-ab97-4904-8c95-282479482133" />

### Принципиальная схема

<img width="1015" height="526" alt="image" src="https://github.com/user-attachments/assets/22afd267-00f7-4aef-ad0c-3efa6e882071" />

### Код программы 

```
5.	#include <stdint.h>
6.	#include "main.h"
7.	
8.	void initClk(void)
9.	{
10.		// Enable HSI
11.		RCC->CR |= RCC_CR_HSION;
12.		while(!(RCC->CR & RCC_CR_HSIRDY)){};
13.	
14.		// Enable Prefetch Buffer
15.		FLASH->ACR |= FLASH_ACR_PRFTBE;
16.	
17.		// Flash 2 wait state
18.		FLASH->ACR &= ~FLASH_ACR_LATENCY;
19.		FLASH->ACR |= FLASH_ACR_LATENCY_2;
20.	
21.		// HCLK = SYSCLK
22.		RCC->CFGR |= RCC_CFGR_HPRE_DIV1;
23.	
24.		// PCLK2 = HCLK
25.		RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;
26.	
27.		// PCLK1 = HCLK
28.		RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;
29.	
30.		// PLL configuration: PLLCLK = HSI/2 * 16 = 64 MHz
31.		RCC->CFGR &= ~RCC_CFGR_PLLSRC;
32.		RCC->CFGR |= RCC_CFGR_PLLMULL16;
33.	
34.		// Enable PLL
35.		RCC->CR |= RCC_CR_PLLON;
36.	
37.		// Wait till PLL is ready
38.		while((RCC->CR & RCC_CR_PLLRDY) == 0) {};
39.	
40.		// Select PLL as system clock source
41.		RCC->CFGR &= ~RCC_CFGR_SW;
42.		RCC->CFGR |= RCC_CFGR_SW_PLL;
43.	
44.		// Wait till PLL is used as system clock source
45.		while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL){};
46.	}
47.	
48.	void delay(uint32_t delay_value)
49.	{
50.		for(uint32_t i = 0; i<delay_value; i++);
51.	}
52.	
53.	void init_ports(void)
54.	{
55.		RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN;
56.		GPIOB->CRL |= (GPIO_CRL_MODE0 | GPIO_CRL_MODE1 | 
57.	      		GPIO_CRL_MODE2 | GPIO_CRL_MODE3 | GPIO_CRL_MODE4 | 
58.	             GPIO_CRL_MODE5 | GPIO_CRL_MODE6 | GPIO_CRL_MODE7);
59.	
60.		GPIOB->CRH |= GPIO_CRH_MODE13 | GPIO_CRH_MODE14;
61.		GPIOB->CRH &= ~(GPIO_CRH_CNF13 | GPIO_CRH_CNF14);
62.	
63.		GPIOB->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_CNF1 | GPIO_CRL_CNF2 |         
64.	      		GPIO_CRL_CNF3 | GPIO_CRL_CNF4 | GPIO_CRL_CNF5 | 
65.	            	GPIO_CRL_CNF6 | GPIO_CRL_CNF7); //(0011) Push-Pull, Output 50MHz								
66.		GPIOC->CRL |= GPIO_CRL_MODE0 | GPIO_CRL_MODE1 | GPIO_CRL_MODE2 |  
67.	      		GPIO_CRL_MODE3;	
68.		GPIOC->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_CNF1 | GPIO_CRL_CNF2 |    
69.	      		GPIO_CRL_CNF3 );	//(0011) Push-Pull, Output 50MHz
70.	}
71.	
72.	void init_tim2(void)
73.	{
74.		RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
75.		TIM2->CR1 |= TIM_CR1_URS;
76.		TIM2->PSC = 8000-1;
77.		TIM2->ARR = 10;
78.		TIM2->CR1 |= TIM_CR1_CEN;
79.	}
80.	
81.	void init_tim3(void)
82.	{
83.		RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
84.		TIM3->CR1 |= TIM_CR1_URS;
85.		TIM3->DIER |= TIM_DIER_UIE;
86.		TIM3->PSC = 32000-1;
87.		TIM3->ARR = 1000;
88.		TIM3->CR1 |= TIM_CR1_CEN;
89.	
90.		NVIC_EnableIRQ(TIM3_IRQn);
91.		NVIC_SetPriority(TIM3_IRQn, 1);
92.	}
93.	
94.	void init_adc(void)
95.	{
96.		RCC->APB2ENR |= RCC_APB2ENR_ADC1EN | RCC_APB2ENR_IOPAEN;
97.		GPIOA->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_MODE0); //PC5
98.		ADC1->SMPR2 |= ADC_SMPR2_SMP0;
99.		ADC1->CR2 |= ADC_CR2_EXTSEL | ADC_CR2_EXTTRIG | ADC_CR2_ADON;
100.	delay(2);
101.	ADC1->CR2 |= ADC_CR2_CAL;
102.	while((ADC1->CR2 & ADC_CR2_CAL) != 0);
103.	
104.	}
105.	
106.	uint16_t adc = 0;
107.	uint8_t value[4] = {0};
108.	
109.	
110.	void TIM3_IRQHandler()
111.	{
112.		TIM3->SR &= ~TIM_SR_UIF;
113.		convert();
114.	}
115.	
116.	void indicator(uint8_t num, uint8_t indicator_index)
117.	{
118.		GPIOC->BSRR = GPIO_BSRR_BR0 | GPIO_BSRR_BR1 | GPIO_BSRR_BR2 | 
119.	      		GPIO_BSRR_BR3; //Сброс PC0-PC3
120.		
121.	      	GPIOB->BSRR = GPIO_BSRR_BR0 | GPIO_BSRR_BR1 | GPIO_BSRR_BR2 | 
122.	      		GPIO_BSRR_BR3 | GPIO_BSRR_BR4 | GPIO_BSRR_BR5| 
123.	           	GPIO_BSRR_BR6 | GPIO_BSRR_BR7 | GPIO_BSRR_BR13 | 
124.	       	GPIO_BSRR_BR14; //Сброс PB0-PB7
125.		
126.	      	GPIOB->BSRR = GPIO_BSRR_BS7; //DP
127.	
128.	
129.		//0
130.		if (num == 0)
131.		{
132.			GPIOB->BSRR = GPIO_BSRR_BS0 | GPIO_BSRR_BS1 | GPIO_BSRR_BS2 | 
133.	 			GPIO_BSRR_BS13 | GPIO_BSRR_BS14 | GPIO_BSRR_BS5;//ABCDEF
134.		}
135.	
136.		if (num == 1)
137.		{
138.			GPIOB->BSRR = GPIO_BSRR_BS1 | GPIO_BSRR_BS2;	//BC
139.		}
140.	
141.		if (num == 2)
142.		{
143.			GPIOB->BSRR = GPIO_BSRR_BS0 | GPIO_BSRR_BS1 | GPIO_BSRR_BS13 | 
144.	  			GPIO_BSRR_BS14 | GPIO_BSRR_BS6;	//ABDEG
145.		}
146.	
147.		if (num == 3)
148.		{
149.			GPIOB->BSRR = GPIO_BSRR_BS0 | GPIO_BSRR_BS1 | GPIO_BSRR_BS2 | 
150.	 			GPIO_BSRR_BS13 | GPIO_BSRR_BS6;	//ABCDG
151.		}
152.	
153.		if (num == 4)
154.		{
155.			GPIOB->BSRR = GPIO_BSRR_BS1 | GPIO_BSRR_BS2 | GPIO_BSRR_BS5 | 
156.	 			GPIO_BSRR_BS6;	//BCFG
157.		}
158.	
159.		if (num == 5)
160.		{
161.			GPIOB->BSRR = GPIO_BSRR_BS0 | GPIO_BSRR_BS2 | GPIO_BSRR_BS13 | 
162.	 			GPIO_BSRR_BS5 | GPIO_BSRR_BS6;	//ACDFG
163.		}
164.	
165.		if (num == 6)
166.		{
167.			GPIOB->BSRR = GPIO_BSRR_BS0 | GPIO_BSRR_BS2 | GPIO_BSRR_BS13 |
168.				GPIO_BSRR_BS14 | GPIO_BSRR_BS5 | 
169.	 			GPIO_BSRR_BS6;//ACDEFG
170.		}
171.	
172.		if (num == 7)
173.		{
174.			GPIOB->BSRR = GPIO_BSRR_BS0 | GPIO_BSRR_BS1 | 
175.	     			GPIO_BSRR_BS2;//ABC
176.		}
177.	
178.		if (num == 8)
179.		{
180.			GPIOB->BSRR = GPIO_BSRR_BS0 | GPIO_BSRR_BS1 | GPIO_BSRR_BS2 | 
181.	 			GPIO_BSRR_BS13 | GPIO_BSRR_BS14 | GPIO_BSRR_BS5 | 
182.	 			GPIO_BSRR_BS6;//ABCDEFG
183.		}
184.	
185.		if (num == 9)
186.		{
187.			GPIOB->BSRR = GPIO_BSRR_BS0 | GPIO_BSRR_BS1 | GPIO_BSRR_BS2 |
188.				GPIO_BSRR_BS13 | GPIO_BSRR_BS5 | GPIO_BSRR_BS6;//ABCDFG
189.		}
190.	
191.		if(indicator_index == 0)
192.				GPIOC->BSRR = GPIO_BSRR_BS0;
193.		if(indicator_index == 1)
194.				GPIOC->BSRR = GPIO_BSRR_BS1;
195.		if(indicator_index == 2)
196.				GPIOC->BSRR = GPIO_BSRR_BS2;
197.		if(indicator_index == 3)
198.				GPIOC->BSRR = GPIO_BSRR_BS3;
199.	
200.	}
201.	
202.	uint16_t read_adc(uint8_t channel)
203.	{
204.		ADC1->SQR3 |= channel & (0b11111);
205.		ADC1->CR2 |= ADC_CR2_SWSTART;
206.		while((ADC1->SR & ADC_SR_EOC)==0);
207.		return ADC1->DR;
208.	}
209.	
210.	
211.	
212.	void convert(void)
213.	{
214.		uint8_t length;
215.		uint32_t a;
216.		adc = read_adc(0);
217.		adc -= 40;
218.		a = adc * 375;
219.		for(uint8_t index = 0; index <= 3; index++)
220.		{
221.			length = a/409600;
222.			if (length == 0)
223.			{
224.				a *= 10;
225.				value[index] = length;
226.			}
227.			else
228.			{
229.				value[index] = length;
230.				a -= length*409600;
231.				a *= 10;
232.			}
233.		}
234.	}
235.	
236.	
237.	void indication(void)
238.	{
239.		uint8_t num_i = 0;
240.		while (true)
241.		if((TIM2->SR & TIM_SR_UIF) != 0)
242.		{
243.				indicator(value[num_i], num_i);
244.				TIM2->SR &= ~TIM_SR_UIF;
245.				num_i++;
246.				if (num_i == 4)
247.					num_i = 0;
248.		}
249.	}
250.	
251.	int main(void)
252.	{
253.	
254.		initClk();
255.		init_ports();
256.		init_tim2();
257.		init_tim3();
258.		init_adc();
259.		indication();
260.	    /* Loop forever */
261.		for(;;);
262.	}

```

### Проверка работоспособности

Грубой заменой резистивного датчика линейного перемещения служит переменный резистор. На фото ниже можно заметить, что при предельно крайнем положении ручки, сопротивление резистора равно 31,4 Ом, максимальное же значение составляет примерно 4,5 кОм.

Проверка нуля переменного резистора

<img width="883" height="663" alt="image" src="https://github.com/user-attachments/assets/ab0f4b65-8448-4881-a490-3a86e4a9418e" />

Крайнее положение ручки потенциометра 

<img width="856" height="642" alt="image" src="https://github.com/user-attachments/assets/15130991-e440-4bec-81e2-82ea1ee9b9ec" />

Получение 25см на индикаторе

<img width="844" height="760" alt="image" src="https://github.com/user-attachments/assets/dfa695e8-6d61-4172-b717-cfcea70c48ee" />

Проверка сопротивления при значении 25 см.

<img width="844" height="633" alt="image" src="https://github.com/user-attachments/assets/f29f5cb5-b9a8-4e91-8994-3c8c8b024e4f" />



