enum class RunMode {WithoutRef, WithRef};

const double ThresholdRef = 20;   // Refありなしを判定するためのしきい値 (pe)
const int FitRangeGausLeft = 10;
const int FitRangeGausRight = 10;


const int UsingCh = 10;
const int NRebin = 32;
const int OverFlowCut = 4085;

const double AdcPedestal = 223.8;
const double Adc1pe = 273.8;
const double Gain = Adc1pe - AdcPedestal;
