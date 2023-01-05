import unittest
import chess
import chess.engine
import secrets
import os
import stat
import dataclasses
from typing import Optional, Dict


def debug_command(board: chess.Board, cmd='eval', indeze='Term'):
    @dataclasses.dataclass
    class EvaluationScore:
        mg: Optional[float]
        eg: Optional[float]

    @dataclasses.dataclass
    class EvaluationResult:
        raw: str
        info: Dict[str, Dict[str, EvaluationScore]]

    class UciEvalCommand(chess.engine.BaseCommand[chess.engine.UciProtocol, EvaluationResult]):
        def start(self, engine: chess.engine.UciProtocol) -> None:
            self.eval_result = ''
            self.getting_eval = False
            self.closing_up = False
            self._readyok(engine)

        @staticmethod
        def parse(s: str):
            def _parse_score(t: str) -> Optional[EvaluationScore]:
                try:
                    t = [None if x[-1] == '-' else float(x) for x in t.split(' ') if x]
                except ValueError:
                    t = [None, None]
                assert len(t) == 2
                return EvaluationScore(mg=t[0], eg=t[1])

            rows = [[' '.join(y.strip().split()) for y in x.split('|') if y] for x in s.split('\n') if x and x[0] == '|']
            headers = rows[0]
            last_key = None
            d: Dict[str, Dict[str, EvaluationScore]] = {}
            for k, cols in enumerate(rows):
                if cols[0] == '' or cols[0] == indeze:
                    continue
                if len(cols) > len(headers):
                    if last_key is None:
                        q = {header: cols[i] for i, header in enumerate(headers) if cols[i]}
                        key = q[indeze]
                        last_key = key
                        del q[indeze]
                    else:
                        q = {header: cols[i] for i, header in enumerate(rows[k-1]) if cols[i]}
                        key = last_key
                        last_key = None
                        del q[key]
                else:
                    q = {header: cols[i] for i, header in enumerate(headers) if cols[i]}
                    key = q[indeze]
                    del q[indeze]
                d[key] = {k: _parse_score(v) for k, v in q.items()}
            return d

        def line_received(self, engine: chess.engine.UciProtocol, line: str) -> None:
            if self.closing_up:
                self.result.set_result(EvaluationResult(raw=self.eval_result, info=self.parse(self.eval_result)))
                self.set_finished()
            elif line == '':
                pass
            elif line.startswith('info string variant') or line.startswith('info string classical evaluation'):
                self.getting_eval = True
            elif self.getting_eval:
                self.eval_result += line + '\n'
                if line.startswith('Final'):
                    self.closing_up = True
            else:
                chess.engine.LOGGER.warning("%s: Unexpected engine output: %r", engine, line)

        def _readyok(self, engine: chess.engine.UciProtocol) -> None:
            engine._position(board)
            engine.send_line(cmd)

    return UciEvalCommand


def create_cls(file_path: str, name: str):
    def _str(self):
        return '[{}] {}'.format(name, self._testMethodName)

    def setUp(self):
        self.engine = chess.engine.SimpleEngine.popen_uci(file_path)

    def tearDown(self):
        self.engine.close()

    @classmethod
    def setUpClass(cls):
        cls.temporaryFiles = []

    @classmethod
    def tearDownClass(cls):
        for file in cls.temporaryFiles:
            os.remove(file)

    def write_temporary_file(self, s: str) -> str:
        while True:
            file_name = 'tmp_' + secrets.token_hex(nbytes=16)
            if not os.path.exists(file_name):
                break
        with open(file_name, 'w') as f:
            f.write(s)
        self.temporaryFiles.append(file_name)
        return file_name

    def test_debug(self):
        board = chess.Board('5rk1/ppp1pp2/4qb1p/1P1r2p1/5n2/1Q4B1/P3BPPP/RN2K2R w KQ - 2 18')

        config = '''
scoreValueMg = passed:210 k:0 kingAttackP:410 threatMinorB:610
scoreValueEg = passed:310 k:0 kingAttackN:510 threatMinorR:710
'''

        file_name = self.write_temporary_file('[human:chess]\n' + config)

        class Variant(chess.Board):
            uci_variant = 'human'
        
        board = Variant(board.fen())
        self.engine.options['UCI_Variant'].var.append('human')

        self.engine.configure({
            'VariantPath': os.path.join(dir_path, file_name),
        })

        r = self.engine.communicate(debug_command(board=board, cmd='variant'))
        self.assertEqual(2.1, r.info['Passed']['MG EG'].mg)
        self.assertEqual(3.1, r.info['Passed']['MG EG'].eg)
        self.assertEqual(4.1, r.info['KingAttack']['P'].mg)
        self.assertEqual(5.1, r.info['KingAttack']['N'].eg)
        self.assertEqual(6.1, r.info['ThreatMinor']['B'].mg)
        self.assertEqual(7.1, r.info['ThreatMinor']['R'].eg)
    
    def test_eval(self):
        board = chess.Board('5rk1/ppp1pp2/4qb1p/1P1r2p1/5n2/1Q4B1/P3BPPP/RN2K2R w KQ - 2 18')

        config = '''
pieceValueMg = p:100 n:300 b:300 r:500 q:900
pieceValueEg = p:100 n:300 b:300 r:500 q:900
scoreValueMg = material:100 p:0 n:0 b:0 r:0 q:0 k:0 imbalance:0 mobility:0 threat:0 passed:0 space:0 variant:0 winnable:0
scoreValueEg = material:100 p:0 n:0 b:0 r:0 q:0 k:0 imbalance:0 mobility:0 threat:0 passed:0 space:0 variant:0 winnable:0
'''

        file_name = self.write_temporary_file('[human:chess]\n' + config)

        class Variant(chess.Board):
            uci_variant = 'human'
        
        board = Variant(board.fen())
        self.engine.options['UCI_Variant'].var.append('human')

        self.engine.configure({
            'VariantPath': os.path.join(dir_path, file_name),
        })

        r = self.engine.communicate(debug_command(board=board))
        print(r.raw)
        self.assertEqual(True, True)
    
    # creating class dynamically
    return type(name, (unittest.TestCase, ), {
    **{ # built-ins
        '__str__': _str,
        'setUp': setUp,
        'setUpClass': setUpClass,
        'tearDown': tearDown,
        'tearDownClass': tearDownClass,
        'write_temporary_file': write_temporary_file,
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
