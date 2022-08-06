# 5words

> Look mummy, no graphs!

This repo contains my attempt at solving the [five-words puzzle presented on 2022-08-03 by Matt Parker of Stand-up Maths](https://www.youtube.com/watch?v=_-AfhLQfb6w) (not the official name of the problem and not the first time it's been discovered, but this is where I saw it first).

My solution is to convert each word into a 32-bit integer (bit vector) and then use the first 27 bits to encode the presence of each letter in the word.
This means that anagrams encode to the same integer.

Then, I recursively build a tree containing the valid 5-word combinations by applying the following pseudocode:

```
Let there be a root node containing the bitmask ~0, 
  an array of encoded words (encodings),
  an empty red-black tree that stores nodes by their bitmasks
  and a list of compatible 5-word sequences (sucesses) stored by computation leaf nodes.

BuildTree(parent, bitmask, depth) -> Node:
  let current be a new node with an empty list of children.
  let parent and bitmask be associated with current.

  if bitmask exists in precomputed nodes:
    return existing node //Meaning that the current branch of computation shortcuts to an existing ending.
  
  if depth is 5:
    add the current node to the successes.
  
  for each encoding:
    if (bitmask & encoding) == encoding: //Note that this is bitwise-AND
      call BuildTree(current, bitmask - encoding, depth + 1) and store the resulting node in current's list of children.
  
  add current to the red-black tree.
  return current.
```

The red-black tree ensures that previous computations are stored in O(n)-space, can be inserted in O(1) and accessed in O(log(n)), providing cheap shortcuts for the program.
The successes-list can be used to print the results once the tree has been built.

I'm bad at explaining this, but for me, working my way back from the finished result of having covered the entire alphabet (technically incorrect, but it shouldn't matter) by performing 'legal' subtractions seemed much easier than trying to stack 5 layers of word combinations.

The solution is provably faster, especially compared to the famous Parker-Algorithm, but the only thing I can directly point to is re-using precomputed results like in a dynamic programming algorithm.

In the end, I get the wrong amount of 5-word combinations, so I must have made a mistake somewhere. Consider this no more than an attempt at solving the problem.

I'd like to extend a thanks to (`fgoncalves` on github)[https://github.com/fgoncalves] for hosting a public implementation of a generic(ish) red-black tree in C.

## Addendum

I wrote an extremely short python script that uses the package PyDictionary to filter out all the words that have no dictionary-defined meaning.
I've left those words in the defined.txt file, but they don't seem to be useful for 5-words...

