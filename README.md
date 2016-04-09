# spellchecker
What this program do?
1. The program takes two arguments: the corpus file and the input to spell-check (the last
argument can be either a document file or a single word).
2. Tokenizes the corpus file and counts the word frequencies to build the model.
3. Tokenizes the input file (or use the single word) and identifies the misspellings, if any.
4. For each misspelling, finds the most likely corrections from the model.
5. Prints each misspelling and its suggested corrections.
