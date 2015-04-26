/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// QuantLib Black-Scholes-Merton test

/*!
Copyright (C) 2005, 2006, 2007 StatPro Italia srl
Copyright (C) 2008 Bojan Nikolic

Further edited Neil L. Burman - 05-Apr-2015
Removed all code except that for Black-Scholes-Merton.
Added comments.

This file is part of QuantLib, a free-software/open-source library
for financial quantitative analysts and developers - http://quantlib.org/

QuantLib is free software: you can redistribute it and/or modify it
under the terms of the QuantLib license.  You should have received a
copy of the license along with this program; if not, please email
<quantlib-dev@lists.sf.net>. The license is also available online at
<http://quantlib.org/license.shtml>.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the license for more details.
*/


// The only header you need to use QuantLib
#include <ql/quantlib.hpp>

// Boost and other headers
#include <boost/timer.hpp>
#include <boost/variant.hpp>
#include <iostream>
#include <iomanip>

using namespace QuantLib;

// Input data
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

// Print the input values
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


// Ask the user to press <Enter> to continue program flow
void PressEnter()
{
	std::cout << "\nPress <Enter> to continue...";
	std::cin.ignore();
}

// Print a row of output data
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

// Set up the pricing engine
void BlackScholes(VanillaOption & euro,
	boost::shared_ptr<BlackScholesMertonProcess> bsmProcess)
{

	boost::shared_ptr<PricingEngine> pe(new AnalyticEuropeanEngine(bsmProcess));

	euro.setPricingEngine(pe);

	PrintResRow("Black-Scholes",
		euro.NPV());
}

// Set up the option parameters
void EquityOption(void)
{

	std::cout << std::endl;

	// Set up dates
	Calendar calendar = TARGET();
	Date todaysDate(15, May, 1998);
	Date settlementDate(17, May, 1998);
	Settings::instance().evaluationDate() = todaysDate;

	// Our options
	OptionInputs in;

	// Set up the option parameters
	in.type = Option::Put;
	in.underlying = 36;
	in.strike = 40;
	in.dividendYield = 0.00;
	in.riskFreeRate = 0.06;
	in.volatility = 0.20;
	in.maturity = Date(17, May, 1999);
	in.dayCounter = Actual365Fixed();

	// Print the options
	PrintInputs(std::cout, in);
	std::cout << std::endl;

	// Print "today's date"
	std::cout << "Today's Date : " << todaysDate << std::endl << std::endl;

	// write column headings
	PrintResRow("Method",
		"European");

	boost::shared_ptr<Exercise> europeanExercise(
		new EuropeanExercise(in.maturity));

	Handle<Quote> underlyingH(
		boost::shared_ptr<Quote>(new SimpleQuote(in.underlying)));

	// Bootstrap the yield/dividend/vol curves
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

	// Set up the Black-Scholes-Merton process
	boost::shared_ptr<BlackScholesMertonProcess> bsmProcess(
		new BlackScholesMertonProcess(underlyingH,
		flatDividendTS,
		flatTermStructure,
		flatVolTS));

	// Vanilla option - European
	VanillaOption europeanOption(payoff, europeanExercise);

	// Analytic formulas:

	// Black-Scholes for European
	BlackScholes(europeanOption,
		bsmProcess);

}

// Get the option price and print timing information
int main(int, char*[]) {

	try {

		// Start the timer
		boost::timer timer;

		// Price the option
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

		// Output the elapsed time
		std::cout << std::fixed << std::setprecision(5)
			<< seconds << " s\n" << std::endl;

		// Ask the user to continue
		PressEnter();

		// Exit
		return 0;
	}

	// Known unknown exceptions
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
		return 1;
	}

	// Unknown unknown exceptions
	catch (...) {
		std::cout << "unknown error" << std::endl;
		return 1;
	}
}