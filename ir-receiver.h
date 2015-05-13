// ir-receiver.h
//

// Power remote with Spark Core (3.3V*)
// Connect A0 to IR LED cathode (-)

#define IR_PIN				A0

// KDL-50W790B
#define IR_PERIOD			24.947
#define IR_COUNT			0//13 - 1
#define IR_THRESHOLD		3
#define IR_LIMIT			100

const uint16_t _pc[][2] =
{
	{ 97, 24 },
	{ 24, 24 },
	{ 48, 24 },
	{ 0, 0 },
};

// GWC18KG-D3DNA6C/I
//#define IR_PERIOD			26.701632
//#define IR_COUNT			70 - 1
//#define IR_THRESHOLD		3
//#define IR_LIMIT			100
//
//const uint16_t _pc[][2] =
//{
//	{ 336, 168 },
//	{  23,  22 },
//	{  23,  63 },
//	{  23, 745 },
//	{   0,   0 },
//};

// VIP2262
//#define IR_PERIOD			27.586206
//#define IR_COUNT			18 - 1
//#define IR_THRESHOLD		3
//#define IR_LIMIT			100
//
//const uint16_t _pc[][2] =
//{
//	{ 15, 10 },
//	{ 6, 10 },
//	{ 6, 16 },
//	{ 6, 22 },
//	{ 6, 28 },
//	{ 0, 0 },
//};

// HWC560S
//#define IR_PERIOD			26.473936
//#define IR_COUNT			50 - 1
//#define IR_THRESHOLD		3
//#define IR_LIMIT			100
//
//const uint16_t _pc[][2] =
//{
//	{ 94, 72 },
//	{ 16, 16 },
//	{ 16, 44 },
//};

namespace Receiver
{
	volatile uint32_t _v = 0;
	volatile uint32_t _t = 0;

	uint32_t _ts[512] = { 0 };
	uint8_t _tt[512] = { 0 };
	uint32_t t = 0;

	class Program
	{
	public:
		static void Setup()
		{
			Serial.begin(115200);
			while (!Serial.available());

			Serial.println("[F] Flash\r\n[D] Dump Buffer\r\n");

			pinMode(IR_PIN, INPUT_PULLDOWN);
			attachInterrupt(IR_PIN, _ISR, RISING);
		}

		static void Loop()
		{
			if (Serial.available())
			{
				uint8_t k = Serial.read();
				switch (k)
				{
					case 'f':
						System.bootloader();
						break;
					case 'd':
						_Dump();
						break;
					default:
						break;
				}
			}

			if (IR_COUNT && _v >= IR_COUNT * 2)
				_Dump();
		}

	private:
		static void _ISR()
		{
			if (_v < sizeof(_ts) / sizeof(uint32_t))
			{
				t = micros() - _t;

				if (!_t || t < IR_LIMIT)
				{
					_tt[_v] = 'M';
					++(_ts[_v]);
				}
				else
				{
					_tt[++_v] = 'S';
					_ts[_v++] = t;
				}
			}

			_t = micros();
		}

		static void _Dump()
		{
			for (uint32_t i = 0; i < _v; i++)
			{
				if (i % 2)
				{
					uint32_t m = _ts[i - 1];
					uint32_t s = (uint32_t)(_ts[i] / IR_PERIOD + 0.5);

					const uint16_t(*p)[2] = _pc;
					while ((*p)[0] && (*p)[1])
					{
						if ((m >(*p)[0] - IR_THRESHOLD && m < (*p)[0] + IR_THRESHOLD) &&
							(s >(*p)[1] - IR_THRESHOLD && s < (*p)[1] + IR_THRESHOLD))
						{
							Serial.print((char)('0' + (p - _pc)));
							break;
						}

						++p;
					}

					if (!(*p)[0] && !(*p)[1])
					{
						Serial.print("M");
						Serial.print(m);
						Serial.print(":S");
						Serial.println(s);
					}
				}
			}

			Serial.println();

			for (uint32_t i = 0; i < _v; i++)
				_ts[i] = _tt[i] = 0;

			_v = _t = 0;
		}
	};
}
