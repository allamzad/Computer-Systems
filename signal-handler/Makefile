CC = clang
CFLAGS = -Wall -Wextra -g -fno-omit-frame-pointer

TESTS_1 =            hello_no_malloc hello_malloc_free hello_free_null random_trace
TESTS_2 = $(TESTS_1) hello_double_free random_trace_double_free
TESTS_3 = $(TESTS_2) hello_invalid_free1 hello_invalid_free2 hello_invalid_free3 \
                     hello_invalid_free4 hello_invalid_free5 \
                     hello_invalid_free6 hello_invalid_free7
TESTS_4 = $(TESTS_3) hello_leak random_trace_leak1 random_trace_leak10
TESTS_5 = $(TESTS_4) hello_access_null \
                     hello_overflow1 hello_overflow2 hello_overflow3 hello_overflow4 \
                     hello_use_after_free1 hello_use_after_free2 hello_use_after_free3 \
                     hello_use_after_free4 hello_use_after_free5 random_trace_use_after_free \
                     hello_access_header1 hello_access_header2 hello_access_footer \
                     hello_access_before_heap hello_access_after_heap hello_header_after_free

test: test5
test1: $(TESTS_1:=-result)
test2: $(TESTS_2:=-result)
test3: $(TESTS_3:=-result)
test4: $(TESTS_4:=-result)
test5: $(TESTS_5:=-result)

bin/%.o: %.c
	$(CC) $(CFLAGS) -c -fpic $^ -o $@

bin/%.o: tests/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

bin/libmyasan.so: bin/myasan.o bin/util.o
	$(CC) $(CFLAGS) -shared -fpic $^ -o $@

bin/%: bin/%.o bin/libmyasan.so
	$(CC) $(CFLAGS) -rdynamic $^ -o $@

tests/%-expected-code.txt: tests/%.c
	grep -E '// [0-9]+' $^ | sed 's/\/\/ //' > $@

tests/%-expected-stdout.txt: tests/%.c
	grep '// stdout: ' $^ | sed 's/\/\/ stdout: //' > $@

tests/%-expected-stderr.txt: tests/%.c
	grep '// stderr: ' $^ | sed 's/\/\/ stderr: //' > $@

tests/%-actual-code.txt: bin/%
	$^ > $(@:code.txt=stdout.txt) 2> $(@:code.txt=stderr.txt); \
		code=$$?; \
		if [ $$code -gt 30 ]; then \
			cat $(@:code.txt=stdout.txt) $(@:code.txt=stderr.txt); \
			exit $$code; \
		fi; \
		echo $$code > $@

%-result: tests/%-actual-code.txt tests/%-expected-code.txt tests/%-expected-stdout.txt tests/%-expected-stderr.txt
	diff -u $(word 3,$^) $(<:code.txt=stdout.txt) && \
		diff -u $(word 4,$^) $(<:code.txt=stderr.txt) && \
		diff -u $(word 2,$^) $< && \
		echo "\e[32mPASSED test $(@:-result=)!\e[39m" || \
		(echo "\e[31mFAILED test $(@:-result=)\e[39m"; false)

clean:
	rm -f bin/* tests/*.txt

.PRECIOUS: bin/%.o bin/libmyasan.so bin/% \
	tests/%-expected-code.txt tests/%-expected-stdout.txt tests/%-expected-stderr.txt \
	tests/%-actual-code.txt
