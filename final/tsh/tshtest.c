/*
 * Non-interactive TSH regression test.
 *
 * This replaces the original menu-driven tester for the final project so the
 * tuple API and OP_SHELL service can be verified with expected results.
 */

#include "tshtest.h"

static int failures = 0;

static void expect(int condition, const char *name)
{
   printf("%s: %s\n", condition ? "PASS" : "FAIL", name);
   if (!condition)
      failures++;
}

static void expectTuple(const char *label, const char *expected_name,
                        const char *expected_data, int expected_priority,
                        char *actual_name, void *tuple,
                        int length, int priority)
{
   int ok;

   ok = strcmp(actual_name, expected_name) == 0 &&
        length == (int)strlen(expected_data) + 1 &&
        priority == expected_priority &&
        strcmp((char *)tuple, expected_data) == 0;

   expect(ok, label);
}

int main(int argc, char **argv)
{
   u_short port;
   char tuple_name[TUPLENAME_LEN];
   char tpname[TUPLENAME_LEN];
   char output[256];
   char *pipe_count;
   void *tuple = NULL;
   int length = 0;
   int priority = 0;

   if (argc < 2)
   {
      printf("Usage: %s port\n", argv[0]);
      return 1;
   }

   port = (u_short)atoi(argv[1]);
   snprintf(tuple_name, sizeof(tuple_name), "UNIT_%ld", (long)getpid());

   expect(tsh_put(port, tuple_name, "world", 6, 7), "tsh_put stores tuple");

   if (tsh_read(port, tuple_name, tpname, &tuple, &length, &priority))
   {
      expectTuple("tsh_read returns expected tuple without removing it",
                  tuple_name, "world", 7, tpname, tuple, length, priority);
      free(tuple);
      tuple = NULL;
   }
   else
   {
      expect(0, "tsh_read returns expected tuple without removing it");
   }

   if (tsh_get(port, tuple_name, tpname, &tuple, &length, &priority))
   {
      expectTuple("tsh_get returns expected tuple",
                  tuple_name, "world", 7, tpname, tuple, length, priority);
      free(tuple);
      tuple = NULL;
   }
   else
   {
      expect(0, "tsh_get returns expected tuple");
   }

   expect(!tsh_read(port, tuple_name, tpname, &tuple, &length, &priority),
          "tsh_get removes tuple from tuple space");

   memset(output, 0, sizeof(output));
   expect(tsh_shell_cmd(port, "printf shell-ok", output, sizeof(output)),
          "OP_SHELL executes foreground command");
   expect(strcmp(output, "shell-ok") == 0,
          "OP_SHELL returns expected foreground output");

   memset(output, 0, sizeof(output));
   expect(tsh_shell_cmd(port, "printf hello | wc -c", output, sizeof(output)),
          "OP_SHELL executes Lab 4 style pipeline");
   pipe_count = output;
   while (*pipe_count == ' ' || *pipe_count == '\t' || *pipe_count == '\n')
      pipe_count++;
   expect(strncmp(pipe_count, "5", 1) == 0,
          "OP_SHELL pipeline returns expected output");

   printf("Summary: %d failure(s)\n", failures);
   return failures == 0 ? 0 : 1;
}

