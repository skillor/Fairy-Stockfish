import unittest
import chess
import chess.engine
import os
import stat
import dataclasses
from typing import Optional, Dict


def eval(board: chess.Board):
    @dataclasses.dataclass
    class EvaluationScore:
        mg: Optional[float]
        eg: Optional[float]

    @dataclasses.dataclass
    class EvaluationPovs:
        white: EvaluationScore
        black: EvaluationScore
        total: EvaluationScore

    @dataclasses.dataclass
    class EvaluationResult:
        raw: str
        info: Dict[str, EvaluationPovs]

    class UciEvalCommand(chess.engine.BaseCommand[chess.engine.UciProtocol, EvaluationResult]):
        def start(self, engine: chess.engine.UciProtocol) -> None:
            self.eval_result = ''
            self.getting_eval = False
            self.closing_up = False
            self._readyok(engine)

        @staticmethod
        def parse(s: str):
            def _parse_score(t: str) -> EvaluationScore:
                t = [None if x[0] == '-' else float(x) for x in t.split(' ') if x]
                assert len(t) == 2
                return EvaluationScore(mg=t[0], eg=t[1])

            rows = [x for x in s.split('\n') if x and x[0] == '|'][2:]
            d: Dict[str, EvaluationPovs] = {}
            for row in rows:
                cols = row.split('|')
                key = cols[1].strip()
                d[key] = EvaluationPovs(
                    white=_parse_score(cols[2].strip()),
                    black=_parse_score(cols[3].strip()),
                    total=_parse_score(cols[4].strip()),
                )
            return d

        def line_received(self, engine: chess.engine.UciProtocol, line: str) -> None:
            if self.closing_up:
                self.result.set_result(EvaluationResult(raw=self.eval_result, info=self.parse(self.eval_result)))
                self.set_finished()
            elif line == '':
                pass
            elif line.startswith('info string variant'):
                self.getting_eval = True
            elif self.getting_eval:
                self.eval_result += line + '\n'
                if line.startswith('Final evaluation'):
                    self.closing_up = True
            else:
                chess.engine.LOGGER.warning("%s: Unexpected engine output: %r", engine, line)

        def _readyok(self, engine: chess.engine.UciProtocol) -> None:
            engine._position(board)
            engine.send_line('eval')

    return UciEvalCommand


def create_cls(file_path: str, name: str):
    def _str(self):
        return '[{}] {}'.format(name, self._testMethodName)

    @classmethod
    def setUpClass(cls):
        cls.engine = chess.engine.SimpleEngine.popen_uci(file_path)

    @classmethod
    def tearDownClass(cls):
        cls.engine.close()
    
    def test_eval(self):
        board = chess.Board('1r4k1/5ppp/3Rb3/8/6r1/7K/7P/8 w - - 0 32')

        with open(os.path.join(dir_path, 'temp_variants.ini'), 'w') as f:
            f.write('''[human:chess]
pieceValueMg = p:0 n:0 b:0 r:9000 q:0
pieceValueEg = p:0 n:0 b:0 r:9000 q:0
''')

        class Variant(chess.Board):
            uci_variant = 'human'
        
        board = Variant(board.fen())
        self.engine.options['UCI_Variant'].var.append('human')

        self.engine.configure({
            'VariantPath': os.path.join(dir_path, 'temp_variants.ini'),
        })

        r = self.engine.communicate(eval(board=board))
        print(r.raw)
        sf_info = self.engine.analyse(board, chess.engine.Limit(depth=1), multipv=1)
        print(sf_info)
        # self.engine.ping()
        self.assertEqual(True, True)
    
    # creating class dynamically
    return type(name, (unittest.TestCase, ), {
    **{ # built-ins
        '__str__': _str,
        'setUpClass': setUpClass,
        'tearDownClass': tearDownClass,
    }, # test functions
    **{
        n: x for n, x in locals().items() if n.startswith('test_')
    }})



dir_path = os.path.dirname(os.path.realpath(__file__))
exec_path = os.path.join(dir_path, 'src')
for file_name in os.listdir(exec_path):
    file_path = os.path.join(exec_path, file_name)
    if os.path.isfile(file_path):
        executable = bool(os.stat(file_path)[stat.ST_MODE] & (stat.S_IXUSR|stat.S_IXGRP|stat.S_IXOTH))
        if executable:
            name = os.path.basename(file_name).capitalize()
            globals()['Test{}'.format(name)] = create_cls(file_path=file_path, name=name)


def main():
    unittest.main(verbosity=2)

if __name__ == '__main__':
    main()
