// Reference rule for exercise 19: the simulator "dispatch", Linux and Windows builds.
//
// It does not use the decoded configuration, which is not in the file. It uses what is:
// the plain log templates of the behaviors, and the bytes of the first encoded record.

rule triage_desk_dispatch
{
    meta:
        description = "Harmless simulator of exercise 19 (encoded configuration, five logged steps)"
        technique = "T1543.003, T1574.002, T1071.004, T1070.001, T1057"
        reference = "exercises/19-triage-desk"
    strings:
        $service  = "would create the service <cfg" ascii
        $sideload = "would place the library <cfg" ascii
        $dns      = "would resolve names under <cfg" ascii
        $clear    = "would clear the event log <cfg" ascii
        $discover = "would query the list of running processes" ascii
        $record0  = { 0D 13 A2 64 A9 CB 25 68 90 F4 1A 79 9F 9C 9C }
    condition:
        (uint32(0) == 0x464c457f or uint16(0) == 0x5a4d) and
        3 of ($service, $sideload, $dns, $clear, $discover) and $record0
}
