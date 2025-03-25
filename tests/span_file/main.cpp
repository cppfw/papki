#include "../../src/papki/span_file.hpp"

// NOLINTNEXTLINE(bugprone-exception-escape, "we want uncaught exceptions to fail the tests")
int main(int argc, char *argv[]){
	// test read only span_file
	{
		const auto hw = "Hello world!";

		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
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

	// test span_file spawning
	{
		const auto hw = "Hello world!";

		auto span = utki::make_span(hw);

		papki::span_file file(span);

		file.open(papki::file::mode::read);

		std::array<char, 3> buf{};
		{
			auto res = file.read(utki::to_uint8_t(utki::make_span(buf)));
			utki::assert(res == buf.size(), SL);
		}

		auto file2 = file.spawn();
		utki::assert(file2, SL);

		auto res = file2->load();

		file.close();

		utki::assert(span.size() == res.size(), SL);
		utki::assert(
			utki::deep_equals(
				span,
				utki::make_span(res)
			),
			SL
		);
	}

	return 0;
}
