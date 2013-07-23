#!/usr/bin/env python
# -*- coding:utf-8 -*-

import sys
import argparse
from subprocess import Popen, PIPE


class MappingStatsCombiner:
    def __init__(self, stream, mapper='bowtie'):
        self.stream = stream
        if mapper == 'bowtie':
            self.init_bowtie_combiner()
        
        for line in self.filter_stream():
            self.update(line)

    def init_bowtie_combiner(self):
        self.d = {
            '# reads with at least one reported alignment:': 0,
            '# reads that failed to align:': 0,
            '# reads with alignments suppressed due to -m:': 0
        }
        self.filter_stream = self.bowtie_filter
        self.update = self.bowtie_update_d
        self.__str__ = self.bowtie_output

    def bowtie_filter(self):
        self.stream = self.stream.split('\n')
        for line in (l.strip() for l in self.stream if l.startswith('#')):
            yield line

    def bowtie_update_d(self, line):
        try:
            key = ' '.join(line.strip().split(' ')[:-2])
            val = int(line.strip().split(' ')[-2])
            self.d[key] += val
        except (ValueError, TypeError):
            pass

    def bowtie_output(self):
        t = sum(self.d.values())
        u = self.d['# reads with at least one reported alignment:']
        f = self.d['# reads that failed to align:']
        m = self.d['# reads with alignments suppressed due to -m:']
        return '\n'.join([
            '# reads processed: ' + str(t),
            '# reads with at least one reported alignment: ' + str(u) + ' (' + str(round((u*100.0)/t, 2)) + '%)',
            '# reads that failed to align: ' + str(f) + ' (' + str(round((f*100)/t, 2)) + '%)',
            '# reads with alignments suppressed due to -m: ' + str(m) + ' (' + str(round((m*100.0)/t, 2)) + '%)',
            'Reported ' + str(u) + ' alignments to 1 output stream(s)'
        ])



def main(args):

    cmd = [
        'mpiexec',
        '-f', '/etc/mpich2/hostnames',
        '-n', args.n_nodes
    ]

    if args.task == 'distribute':
        cmd.extend([
            'pmap_dist',
            args.workdir,
            args.outdir,
            args.readfile,
            '-r ' + args.readfile2 if args.readfile2 else ''
        ])
        cmd = ' '.join(map(str, cmd))
        proc = Popen(
            cmd,
            stdout=PIPE,
            stderr=PIPE,
            shell=True
        )
        out, err = proc.communicate()

    if args.task == 'map':
        cmd.extend([
            'pmap',
            '-dc' if args.do_not_cleanup else '',
            '-dry' if args.dry else '',
            '-i', args.index[0], args.index[1],
            args.workdir,
            args.outdir,
            args.mapper,
            args.args
        ])
        cmd = ' '.join(map(str, cmd))
        proc = Popen(
            cmd,
            stdout=PIPE,
            stderr=PIPE,
            shell=True
        )
        out, err = proc.communicate()
        mapping_statistics = MappingStatsCombiner(stream=err, mapper=args.mapper)
        print >>sys.stdout, mapping_statistics


if __name__ == '__main__':
    
    # Shared options
    shared_parser = argparse.ArgumentParser(add_help=False)
    shared_parser.add_argument(
        '-w', '--workdir', dest='workdir',
        default='/tmp/pmap_temp',
        help='Temporary working directory (on each node)'
    )
    shared_parser.add_argument(
        '-o', '--outdir', dest='outdir',
        help='Final output directory on the head node'
    )
    shared_parser.add_argument(
        '-n', '--n_nodes', dest='n_nodes',
        type=int,
        default=1,
        help='Number of nodes to to use'
    )

    # Distribute options
    dist_parser = argparse.ArgumentParser(parents=[shared_parser], add_help=False)
    dist_parser.add_argument(
        '-r', '--readfile', dest='readfile',
        help='Reads file'
    )
    dist_parser.add_argument(
        '-R', '--readfile2', dest='readfile2',
        default=False,
        help='Second reads file [pair-end]'
    )

    # Mapping parser
    map_parser = argparse.ArgumentParser(parents=[shared_parser], add_help=False)
    map_parser.add_argument(
        '-i', '--index', dest='index',
        nargs=2,
        help='Location of the index files and prefix'
    )
    map_parser.add_argument(
        '-m', '--mapper', dest='mapper',
        choices=['bowtie', 'bwa'],
        default='bowtie',
        help='Mapper to use'
    )
    map_parser.add_argument(
        '-a', '--args', dest='args',
        default='',
        help='Additional mapper parameters'
    )
    map_parser.add_argument(
        '-d', '--dry', dest='dry',
        action='store_true',
        default=False,
        help='Dry run, do not save mapped reads'
    )
    map_parser.add_argument(
        '-c', '--do_not_cleanup', dest='do_not_cleanup',
        action='store_true',
        default=False,
        help='Cleanup intermediate files on nodes'
    )

    parser = argparse.ArgumentParser(parents=[shared_parser], formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    subparser = parser.add_subparsers(dest='task')
    subparser.add_parser('distribute', help='Distribute resources accross nodes', parents=[dist_parser], formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    subparser.add_parser('map', help='Mapping', parents=[map_parser], formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    main(parser.parse_args())
