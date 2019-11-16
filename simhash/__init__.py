#! /usr/bin/env python

from .simhash import unsigned_hash, num_differing_bits, compute, find_all
from six.moves import range as six_range


def shingle(tokens, window=4):
    """A generator for a moving window of the provided tokens."""
    if window <= 0:
        raise ValueError("Window size must be positive")

    # Start with an empty output set.
    curr_window = []

    # Iterate over the input tokens, once.
    for token in tokens:
        # Add to the window.
        curr_window.append(token)

        # If we've collected too many, remove the oldest item(s) from the collection
        while len(curr_window) > window:
            curr_window.pop(0)

        # Finally, if the window is full, yield the data set.
        if len(curr_window) == window:
            yield list(curr_window)
