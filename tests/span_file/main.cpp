#include "../../src/papki/span_file.hpp"

int main(int argc, char *argv[]){
	// test read only span_file
	{
		const auto hw = "Hello world!";

		auto span = utki::make_span(reinterpret_cast<const uint8_t*>(hw), strlen(hw));

		papki::span_file file(span);

		auto res = file.load();

		ASSERT_ALWAYS(span.size() == res.size())
		auto i = span.begin();
		auto j = res.begin();
		for(; i != span.end(); ++i, ++j){
			ASSERT_ALWAYS(*i == *j)
		}
	}
	return 0;
}
