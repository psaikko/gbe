import xml.etree.cElementTree as ET
import subprocess
from collections import namedtuple
import os
import sys

print(sys.argv[0])

RUNNER_PATH = "./run_test_rom"

TestSuite = namedtuple("TestSuite", "name type main_rom_path individual_rom_paths")

test_suites = [
    TestSuite("halt_bug", "memory", "../gb-test-roms/halt_bug.gb", []),
    TestSuite("mem_timing-2", "memory", "../gb-test-roms/mem_timing-2/mem_timing.gb", [
        "../gb-test-roms/mem_timing-2/rom_singles/03-modify_timing.gb",
        "../gb-test-roms/mem_timing-2/rom_singles/02-write_timing.gb",
        "../gb-test-roms/mem_timing-2/rom_singles/01-read_timing.gb"
    ]),
    TestSuite("instr_timing", "serial",  "../gb-test-roms/instr_timing/instr_timing.gb", []),
    TestSuite("oam_bug", "memory", "../gb-test-roms/oam_bug/oam_bug.gb", [
        "../gb-test-roms/oam_bug/rom_singles/4-scanline_timing.gb",
        "../gb-test-roms/oam_bug/rom_singles/2-causes.gb",
        "../gb-test-roms/oam_bug/rom_singles/3-non_causes.gb",
        "../gb-test-roms/oam_bug/rom_singles/7-timing_effect.gb",
        "../gb-test-roms/oam_bug/rom_singles/1-lcd_sync.gb",
        "../gb-test-roms/oam_bug/rom_singles/8-instr_effect.gb",
        "../gb-test-roms/oam_bug/rom_singles/5-timing_bug.gb",
        "../gb-test-roms/oam_bug/rom_singles/6-timing_no_bug.gb",
    ]),
    TestSuite("dmg_sound", "memory", "../gb-test-roms/dmg_sound/dmg_sound.gb", [
        "../gb-test-roms/dmg_sound/rom_singles/07-len sweep period sync.gb",
        "../gb-test-roms/dmg_sound/rom_singles/11-regs after power.gb",
        "../gb-test-roms/dmg_sound/rom_singles/08-len ctr during power.gb",
        "../gb-test-roms/dmg_sound/rom_singles/06-overflow on trigger.gb",
        "../gb-test-roms/dmg_sound/rom_singles/03-trigger.gb",
        "../gb-test-roms/dmg_sound/rom_singles/01-registers.gb",
        "../gb-test-roms/dmg_sound/rom_singles/02-len ctr.gb",
        "../gb-test-roms/dmg_sound/rom_singles/10-wave trigger while on.gb",
        "../gb-test-roms/dmg_sound/rom_singles/09-wave read while on.gb",
        "../gb-test-roms/dmg_sound/rom_singles/12-wave write while on.gb",
        "../gb-test-roms/dmg_sound/rom_singles/04-sweep.gb",
        "../gb-test-roms/dmg_sound/rom_singles/05-sweep details.gb",
    ]),
    TestSuite("cpu_instrs", "serial", "../gb-test-roms/cpu_instrs/cpu_instrs.gb", [
        "../gb-test-roms/cpu_instrs/individual/05-op rp.gb",
        "../gb-test-roms/cpu_instrs/individual/10-bit ops.gb",
        "../gb-test-roms/cpu_instrs/individual/02-interrupts.gb",
        "../gb-test-roms/cpu_instrs/individual/08-misc instrs.gb",
        "../gb-test-roms/cpu_instrs/individual/01-special.gb",
        "../gb-test-roms/cpu_instrs/individual/06-ld r,r.gb",
        "../gb-test-roms/cpu_instrs/individual/09-op r,r.gb",
        "../gb-test-roms/cpu_instrs/individual/03-op sp,hl.gb",
        "../gb-test-roms/cpu_instrs/individual/04-op r,imm.gb",
        "../gb-test-roms/cpu_instrs/individual/11-op a,(hl).gb",
        "../gb-test-roms/cpu_instrs/individual/07-jr,jp,call,ret,rst.gb",
    ]),
    # TestSuite("cgb_sound", "memory", "../gb-test-roms/cgb_sound/cgb_sound.gb", [
    #     "../gb-test-roms/cgb_sound/rom_singles/07-len sweep period sync.gb",
    #     "../gb-test-roms/cgb_sound/rom_singles/11-regs after power.gb",
    #     "../gb-test-roms/cgb_sound/rom_singles/08-len ctr during power.gb",
    #     "../gb-test-roms/cgb_sound/rom_singles/06-overflow on trigger.gb",
    #     "../gb-test-roms/cgb_sound/rom_singles/03-trigger.gb",
    #     "../gb-test-roms/cgb_sound/rom_singles/12-wave.gb",
    #     "../gb-test-roms/cgb_sound/rom_singles/01-registers.gb",
    #     "../gb-test-roms/cgb_sound/rom_singles/02-len ctr.gb",
    #     "../gb-test-roms/cgb_sound/rom_singles/10-wave trigger while on.gb",
    #     "../gb-test-roms/cgb_sound/rom_singles/09-wave read while on.gb",
    #     "../gb-test-roms/cgb_sound/rom_singles/04-sweep.gb",
    #     "../gb-test-roms/cgb_sound/rom_singles/05-sweep details.gb",
    # ]),
    TestSuite("interrupt_time", "serial", "../gb-test-roms/interrupt_time/interrupt_time.gb", []),
    TestSuite("mem_timing", "serial", "../gb-test-roms/mem_timing/mem_timing.gb", [
        "../gb-test-roms/mem_timing/individual/03-modify_timing.gb",
        "../gb-test-roms/mem_timing/individual/02-write_timing.gb",
        "../gb-test-roms/mem_timing/individual/01-read_timing.gb",
    ])
]

xml_root = ET.Element("testsuites", name="rom-test-results", time="0")

for ts in test_suites:

    ts_element = ET.SubElement(xml_root, "testsuite",
        name=ts.name,
        tests=str(max(1, len(ts.individual_rom_paths))),
        time="0")

    print(ts.name)
    print(ts.type)

    rom_paths = []

    errors = 0
    failures = 0

    if len(ts.individual_rom_paths):
        rom_paths = ts.individual_rom_paths
    else:
        rom_paths = [ts.main_rom_path]

    for rom_path in rom_paths:
        print(rom_path)

        test_name = os.path.join(*os.path.normpath(rom_path).split(os.sep)[2:])

        tc_element = ET.SubElement(ts_element, "testcase", name=test_name, classname=test_name, time="0")

        proc = subprocess.Popen([RUNNER_PATH, ts.type, rom_path], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out, err = proc.communicate()
        out_str = str(out, "ascii")
        err_str = str(err, "ascii")

        ET.SubElement(tc_element, "system-out").text = repr(out_str)
        ET.SubElement(tc_element, "system-err").text = repr(err_str)

        if proc.returncode != 0:
            if "Failed" in out_str:
                ET.SubElement(tc_element, "failure").text = out_str
                failures += 1
            else:
                ET.SubElement(tc_element, "error").text = err_str
                errors += 1

    ts_element.set('failures', str(failures))
    ts_element.set('errors', str(errors))

tree = ET.ElementTree(xml_root)
tree.write("JUnit.xml")