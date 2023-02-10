#include "../../src/papki/span_file.hpp"

int main(int argc, char *argv[]){
	// test read only span_file
	{
		const auto hw = "Hello world!";

		auto span = utki::make_span(reinterpret_cast<const uint8_t*>(hw), strlen(hw));

		papki::span_file file(span);

		auto res = file.load();

		utki::assert(span.size() == res.size(), SL);
		auto i = span.begin();
		auto j = res.begin();
		for(; i != span.end(); ++i, ++j){
			utki::assert(*i == *j, SL);
		}
	}

	// test const char span file
	{
		const auto hw = "Hello world!";

		auto span = utki::make_span(hw);

		papki::span_file file(span);

		auto res = file.load();

		utki::assert(span.size() == res.size(), SL);
		auto i = span.begin();
		auto j = res.begin();
		for(; i != span.end(); ++i, ++j){
			utki::assert(uint8_t(*i) == *j, SL);
		}
	}
	return 0;
}
