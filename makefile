all: persons talkagent

persons: persons.c
	cc -o persons persons.c

talkagent: talkagent.c
	cc -o talkagent talkagent.c -lpthread
