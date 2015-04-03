
// QuantLib Black-Scholes-Merton test

// the only header you need to use QuantLib
#include <ql/quantlib.hpp>

#include <boost/timer.hpp>
#include <boost/variant.hpp>
#include <iostream>
#include <iomanip>

using namespace QuantLib;

#if defined(QL_ENABLE_SESSIONS)
namespace QuantLib {

	Integer sessionId() { return 0; }

}
#endif

struct OptionInputs {
	Option::Type type;
	Real underlying;
	Real strike;
	Spread dividendYield;
	Rate riskFreeRate;
	Volatility volatility;
	Date maturity;
	DayCounter dayCounter;
};


void PrintInputs(std::ostream & os,
	const OptionInputs &in)
{
	os << "Option type = " << in.type << std::endl;
	os << "Maturity = " << in.maturity << std::endl;
	os << "Underlying price = " << in.underlying << std::endl;
	os << "Strike = " << in.strike << std::endl;
	os << "Risk-free interest rate = " << io::rate(in.riskFreeRate)
		<< std::endl;
	os << "Dividend yield = " << io::rate(in.dividendYield)
		<< std::endl;
	os << "Volatility = " << io::volatility(in.volatility)
		<< std::endl;
	os << "Day Counter = " << in.dayCounter
		<< std::endl;
	os << std::endl;
}

typedef boost::variant<const std::string &, double, const char * > OutputEl;

/** A simple helper class to print the
output variant
*/

class OutputElPrinter :
	public boost::static_visitor<>
{

	std::ostream & os;

public:

	OutputElPrinter(std::ostream & os) :
		os(os)
	{
	}

	void operator()(const std::string & str) const
	{
		os << str;
	}

	void operator()(const char * str) const
	{
		os << str;
	}

	void operator()(double d) const
	{
		os << d;
	}

};


void PrintResRow(const std::string & method,
	OutputEl Euro)
{
	Size widths[] = { 35, 14};
	OutputElPrinter op(std::cout);

	std::cout << std::setw(widths[0]) << std::left << method
		<< std::setw(widths[1]) << std::left;
	boost::apply_visitor(op, Euro);
	std::cout << std::endl;
}

void BlackScholesEx(VanillaOption & euro,
	boost::shared_ptr<BlackScholesMertonProcess> bsmProcess)
{

	boost::shared_ptr<PricingEngine> pe(new AnalyticEuropeanEngine(bsmProcess));

	euro.setPricingEngine(pe);

	PrintResRow("Black-Scholes",
		euro.NPV());
}


void EquityOption(void)
{

	std::cout << std::endl;

	// set up dates
	Calendar calendar = TARGET();
	Date todaysDate(15, May, 1998);
	Date settlementDate(17, May, 1998);
	Settings::instance().evaluationDate() = todaysDate;

	// our options
	OptionInputs in;

	//in.type = Option::Call;

	in.type = Option::Put;
	in.underlying = 36;
	in.strike = 40;
	in.dividendYield = 0.00;
	in.riskFreeRate = 0.06;
	in.volatility = 0.20;
	in.maturity = Date(17, May, 1999);
	in.dayCounter = Actual365Fixed();

	PrintInputs(std::cout, in);
	std::cout << std::endl;

	std::cout << "Today's Date : " << todaysDate << std::endl << std::endl;

	// write column headings
	PrintResRow("Method",
		"European");

	/* Test code
	std::vector<Date> exerciseDates;
	for (Integer i = 1; i <= 4; i++)
		exerciseDates.push_back(settlementDate + 3 * i*Months);

	Integer j = 1;
		std::cout << "exerciseDates[" << j << "]=" << QuantLib::io::long_date(exerciseDates[j]) << "; exerciseDates.size()=" << exerciseDates.size() << std::endl;
	*/

	boost::shared_ptr<Exercise> europeanExercise(
		new EuropeanExercise(in.maturity));

	Handle<Quote> underlyingH(
		boost::shared_ptr<Quote>(new SimpleQuote(in.underlying)));

	// bootstrap the yield/dividend/vol curves
	Handle<YieldTermStructure> flatTermStructure(
		boost::shared_ptr<YieldTermStructure>(
		new FlatForward(settlementDate,
		in.riskFreeRate,
		in.dayCounter)));

	Handle<YieldTermStructure> flatDividendTS(
		boost::shared_ptr<YieldTermStructure>(
		new FlatForward(settlementDate,
		in.dividendYield,
		in.dayCounter)));

	Handle<BlackVolTermStructure> flatVolTS(
		boost::shared_ptr<BlackVolTermStructure>(
		new BlackConstantVol(settlementDate,
		calendar,
		in.volatility,
		in.dayCounter)));

	boost::shared_ptr<StrikedTypePayoff> payoff(
		new PlainVanillaPayoff(in.type,
		in.strike));

	boost::shared_ptr<BlackScholesMertonProcess> bsmProcess(
		new BlackScholesMertonProcess(underlyingH,
		flatDividendTS,
		flatTermStructure,
		flatVolTS));

	// options
	VanillaOption europeanOption(payoff, europeanExercise);

	// Analytic formulas:

	// Black-Scholes for European
	BlackScholesEx(europeanOption,
		bsmProcess);

}

int main(int, char*[]) {

	try {

		// Start the timer
		boost::timer timer;

		EquityOption();

		// Get the elapsed time
		Real seconds = timer.elapsed();
		Integer hours = int(seconds / 3600);
		seconds -= hours * 3600;
		Integer minutes = int(seconds / 60);
		seconds -= minutes * 60;
		std::cout << " \nRun completed in ";
		if (hours > 0)
			std::cout << hours << " h ";
		if (hours > 0 || minutes > 0)
			std::cout << minutes << " m ";
		std::cout << std::fixed << std::setprecision(5)
			<< seconds << " s\n" << std::endl;

		std::cout << "\nPress <Enter> to continue...";
		std::cin.ignore();

		return 0;

	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
		return 1;
	}
	catch (...) {
		std::cout << "unknown error" << std::endl;
		return 1;
	}
}