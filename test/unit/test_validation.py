import unittest
from tools.validate_results import summarize

class TestValidation(unittest.TestCase):
    def row(self, outcome='correct', latency='100'):
        return dict(engine='native', symbol_id='DM01', outcome=outcome, latency_ms=latency)
    def test_screen(self):
        r = summarize([self.row() for _ in range(19)]+[self.row('timeout','')])[0]
        self.assertEqual(r['success_pct'],95)
        self.assertTrue(r['screen_pass'])
    def test_wrong_fails(self):
        self.assertFalse(summarize([self.row() for _ in range(19)]+[self.row('wrong')])[0]['screen_pass'])
    def test_small_sample(self):
        self.assertFalse(summarize([self.row()])[0]['screen_pass'])
    def test_empty(self):
        self.assertEqual(summarize([]),[])
