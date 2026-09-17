import unittest

from verify_target import matches


class MatchTests(unittest.TestCase):
    def test_exact_and_multiple_matches(self):
        self.assertEqual(matches(b"abc-abc", b"abc", "xxx"), [0, 4])

    def test_wildcards_around_longest_anchor(self):
        self.assertEqual(matches(b"00ABCD10--99ABCD11", b"xABCDx", "?xxxx?"), [1, 11])

    def test_all_wildcards(self):
        self.assertEqual(matches(b"abcd", b"00", "??"), [0, 1, 2])

    def test_invalid_mask_length(self):
        with self.assertRaises(ValueError):
            matches(b"abc", b"ab", "x")


if __name__ == "__main__":
    unittest.main()
