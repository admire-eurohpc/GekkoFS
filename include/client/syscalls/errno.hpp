/*
  Copyright 2018-2024, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2024, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  This file is part of GekkoFS' POSIX interface.

  GekkoFS' POSIX interface is free software: you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of the License,
  or (at your option) any later version.

  GekkoFS' POSIX interface is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with GekkoFS' POSIX interface.  If not, see
  <https://www.gnu.org/licenses/>.

  SPDX-License-Identifier: LGPL-3.0-or-later
*/

#ifndef GKFS_SYSCALLS_ERRNO_HPP
#define GKFS_SYSCALLS_ERRNO_HPP

#include <array>
#include <string>

namespace gkfs::syscall {

static const std::array<const char* const, 531> errno_names = {
        /* [  0] = */ NULL,
        /* [  1] = */ "EPERM",
        /* [  2] = */ "ENOENT",
        /* [  3] = */ "ESRCH",
        /* [  4] = */ "EINTR",
        /* [  5] = */ "EIO",
        /* [  6] = */ "ENXIO",
        /* [  7] = */ "E2BIG",
        /* [  8] = */ "ENOEXEC",
        /* [  9] = */ "EBADF",
        /* [ 10] = */ "ECHILD",
        /* [ 11] = */ "EAGAIN",
        /* [ 12] = */ "ENOMEM",
        /* [ 13] = */ "EACCES",
        /* [ 14] = */ "EFAULT",
        /* [ 15] = */ "ENOTBLK",
        /* [ 16] = */ "EBUSY",
        /* [ 17] = */ "EEXIST",
        /* [ 18] = */ "EXDEV",
        /* [ 19] = */ "ENODEV",
        /* [ 20] = */ "ENOTDIR",
        /* [ 21] = */ "EISDIR",
        /* [ 22] = */ "EINVAL",
        /* [ 23] = */ "ENFILE",
        /* [ 24] = */ "EMFILE",
        /* [ 25] = */ "ENOTTY",
        /* [ 26] = */ "ETXTBSY",
        /* [ 27] = */ "EFBIG",
        /* [ 28] = */ "ENOSPC",
        /* [ 29] = */ "ESPIPE",
        /* [ 30] = */ "EROFS",
        /* [ 31] = */ "EMLINK",
        /* [ 32] = */ "EPIPE",
        /* [ 33] = */ "EDOM",
        /* [ 34] = */ "ERANGE",
        /* [ 35] = */ "EDEADLK",
        /* [ 36] = */ "ENAMETOOLONG",
        /* [ 37] = */ "ENOLCK",
        /* [ 38] = */ "ENOSYS",
        /* [ 39] = */ "ENOTEMPTY",
        /* [ 40] = */ "ELOOP",
        /* [ 41] = */ NULL,
        /* [ 42] = */ "ENOMSG",
        /* [ 43] = */ "EIDRM",
        /* [ 44] = */ "ECHRNG",
        /* [ 45] = */ "EL2NSYNC",
        /* [ 46] = */ "EL3HLT",
        /* [ 47] = */ "EL3RST",
        /* [ 48] = */ "ELNRNG",
        /* [ 49] = */ "EUNATCH",
        /* [ 50] = */ "ENOCSI",
        /* [ 51] = */ "EL2HLT",
        /* [ 52] = */ "EBADE",
        /* [ 53] = */ "EBADR",
        /* [ 54] = */ "EXFULL",
        /* [ 55] = */ "ENOANO",
        /* [ 56] = */ "EBADRQC",
        /* [ 57] = */ "EBADSLT",
        /* [ 58] = */ NULL,
        /* [ 59] = */ "EBFONT",
        /* [ 60] = */ "ENOSTR",
        /* [ 61] = */ "ENODATA",
        /* [ 62] = */ "ETIME",
        /* [ 63] = */ "ENOSR",
        /* [ 64] = */ "ENONET",
        /* [ 65] = */ "ENOPKG",
        /* [ 66] = */ "EREMOTE",
        /* [ 67] = */ "ENOLINK",
        /* [ 68] = */ "EADV",
        /* [ 69] = */ "ESRMNT",
        /* [ 70] = */ "ECOMM",
        /* [ 71] = */ "EPROTO",
        /* [ 72] = */ "EMULTIHOP",
        /* [ 73] = */ "EDOTDOT",
        /* [ 74] = */ "EBADMSG",
        /* [ 75] = */ "EOVERFLOW",
        /* [ 76] = */ "ENOTUNIQ",
        /* [ 77] = */ "EBADFD",
        /* [ 78] = */ "EREMCHG",
        /* [ 79] = */ "ELIBACC",
        /* [ 80] = */ "ELIBBAD",
        /* [ 81] = */ "ELIBSCN",
        /* [ 82] = */ "ELIBMAX",
        /* [ 83] = */ "ELIBEXEC",
        /* [ 84] = */ "EILSEQ",
        /* [ 85] = */ "ERESTART",
        /* [ 86] = */ "ESTRPIPE",
        /* [ 87] = */ "EUSERS",
        /* [ 88] = */ "ENOTSOCK",
        /* [ 89] = */ "EDESTADDRREQ",
        /* [ 90] = */ "EMSGSIZE",
        /* [ 91] = */ "EPROTOTYPE",
        /* [ 92] = */ "ENOPROTOOPT",
        /* [ 93] = */ "EPROTONOSUPPORT",
        /* [ 94] = */ "ESOCKTNOSUPPORT",
        /* [ 95] = */ "EOPNOTSUPP",
        /* [ 96] = */ "EPFNOSUPPORT",
        /* [ 97] = */ "EAFNOSUPPORT",
        /* [ 98] = */ "EADDRINUSE",
        /* [ 99] = */ "EADDRNOTAVAIL",
        /* [100] = */ "ENETDOWN",
        /* [101] = */ "ENETUNREACH",
        /* [102] = */ "ENETRESET",
        /* [103] = */ "ECONNABORTED",
        /* [104] = */ "ECONNRESET",
        /* [105] = */ "ENOBUFS",
        /* [106] = */ "EISCONN",
        /* [107] = */ "ENOTCONN",
        /* [108] = */ "ESHUTDOWN",
        /* [109] = */ "ETOOMANYREFS",
        /* [110] = */ "ETIMEDOUT",
        /* [111] = */ "ECONNREFUSED",
        /* [112] = */ "EHOSTDOWN",
        /* [113] = */ "EHOSTUNREACH",
        /* [114] = */ "EALREADY",
        /* [115] = */ "EINPROGRESS",
        /* [116] = */ "ESTALE",
        /* [117] = */ "EUCLEAN",
        /* [118] = */ "ENOTNAM",
        /* [119] = */ "ENAVAIL",
        /* [120] = */ "EISNAM",
        /* [121] = */ "EREMOTEIO",
        /* [122] = */ "EDQUOT",
        /* [123] = */ "ENOMEDIUM",
        /* [124] = */ "EMEDIUMTYPE",
        /* [125] = */ "ECANCELED",
        /* [126] = */ "ENOKEY",
        /* [127] = */ "EKEYEXPIRED",
        /* [128] = */ "EKEYREVOKED",
        /* [129] = */ "EKEYREJECTED",
        /* [130] = */ "EOWNERDEAD",
        /* [131] = */ "ENOTRECOVERABLE",
        /* [132] = */ "ERFKILL",
        /* [133] = */ "EHWPOISON",
        /* [134] = */ NULL,
        /* [135] = */ NULL,
        /* [136] = */ NULL,
        /* [137] = */ NULL,
        /* [138] = */ NULL,
        /* [139] = */ NULL,
        /* [140] = */ NULL,
        /* [141] = */ NULL,
        /* [142] = */ NULL,
        /* [143] = */ NULL,
        /* [144] = */ NULL,
        /* [145] = */ NULL,
        /* [146] = */ NULL,
        /* [147] = */ NULL,
        /* [148] = */ NULL,
        /* [149] = */ NULL,
        /* [150] = */ NULL,
        /* [151] = */ NULL,
        /* [152] = */ NULL,
        /* [153] = */ NULL,
        /* [154] = */ NULL,
        /* [155] = */ NULL,
        /* [156] = */ NULL,
        /* [157] = */ NULL,
        /* [158] = */ NULL,
        /* [159] = */ NULL,
        /* [160] = */ NULL,
        /* [161] = */ NULL,
        /* [162] = */ NULL,
        /* [163] = */ NULL,
        /* [164] = */ NULL,
        /* [165] = */ NULL,
        /* [166] = */ NULL,
        /* [167] = */ NULL,
        /* [168] = */ NULL,
        /* [169] = */ NULL,
        /* [170] = */ NULL,
        /* [171] = */ NULL,
        /* [172] = */ NULL,
        /* [173] = */ NULL,
        /* [174] = */ NULL,
        /* [175] = */ NULL,
        /* [176] = */ NULL,
        /* [177] = */ NULL,
        /* [178] = */ NULL,
        /* [179] = */ NULL,
        /* [180] = */ NULL,
        /* [181] = */ NULL,
        /* [182] = */ NULL,
        /* [183] = */ NULL,
        /* [184] = */ NULL,
        /* [185] = */ NULL,
        /* [186] = */ NULL,
        /* [187] = */ NULL,
        /* [188] = */ NULL,
        /* [189] = */ NULL,
        /* [190] = */ NULL,
        /* [191] = */ NULL,
        /* [192] = */ NULL,
        /* [193] = */ NULL,
        /* [194] = */ NULL,
        /* [195] = */ NULL,
        /* [196] = */ NULL,
        /* [197] = */ NULL,
        /* [198] = */ NULL,
        /* [199] = */ NULL,
        /* [200] = */ NULL,
        /* [201] = */ NULL,
        /* [202] = */ NULL,
        /* [203] = */ NULL,
        /* [204] = */ NULL,
        /* [205] = */ NULL,
        /* [206] = */ NULL,
        /* [207] = */ NULL,
        /* [208] = */ NULL,
        /* [209] = */ NULL,
        /* [210] = */ NULL,
        /* [211] = */ NULL,
        /* [212] = */ NULL,
        /* [213] = */ NULL,
        /* [214] = */ NULL,
        /* [215] = */ NULL,
        /* [216] = */ NULL,
        /* [217] = */ NULL,
        /* [218] = */ NULL,
        /* [219] = */ NULL,
        /* [220] = */ NULL,
        /* [221] = */ NULL,
        /* [222] = */ NULL,
        /* [223] = */ NULL,
        /* [224] = */ NULL,
        /* [225] = */ NULL,
        /* [226] = */ NULL,
        /* [227] = */ NULL,
        /* [228] = */ NULL,
        /* [229] = */ NULL,
        /* [230] = */ NULL,
        /* [231] = */ NULL,
        /* [232] = */ NULL,
        /* [233] = */ NULL,
        /* [234] = */ NULL,
        /* [235] = */ NULL,
        /* [236] = */ NULL,
        /* [237] = */ NULL,
        /* [238] = */ NULL,
        /* [239] = */ NULL,
        /* [240] = */ NULL,
        /* [241] = */ NULL,
        /* [242] = */ NULL,
        /* [243] = */ NULL,
        /* [244] = */ NULL,
        /* [245] = */ NULL,
        /* [246] = */ NULL,
        /* [247] = */ NULL,
        /* [248] = */ NULL,
        /* [249] = */ NULL,
        /* [250] = */ NULL,
        /* [251] = */ NULL,
        /* [252] = */ NULL,
        /* [253] = */ NULL,
        /* [254] = */ NULL,
        /* [255] = */ NULL,
        /* [256] = */ NULL,
        /* [257] = */ NULL,
        /* [258] = */ NULL,
        /* [259] = */ NULL,
        /* [260] = */ NULL,
        /* [261] = */ NULL,
        /* [262] = */ NULL,
        /* [263] = */ NULL,
        /* [264] = */ NULL,
        /* [265] = */ NULL,
        /* [266] = */ NULL,
        /* [267] = */ NULL,
        /* [268] = */ NULL,
        /* [269] = */ NULL,
        /* [270] = */ NULL,
        /* [271] = */ NULL,
        /* [272] = */ NULL,
        /* [273] = */ NULL,
        /* [274] = */ NULL,
        /* [275] = */ NULL,
        /* [276] = */ NULL,
        /* [277] = */ NULL,
        /* [278] = */ NULL,
        /* [279] = */ NULL,
        /* [280] = */ NULL,
        /* [281] = */ NULL,
        /* [282] = */ NULL,
        /* [283] = */ NULL,
        /* [284] = */ NULL,
        /* [285] = */ NULL,
        /* [286] = */ NULL,
        /* [287] = */ NULL,
        /* [288] = */ NULL,
        /* [289] = */ NULL,
        /* [290] = */ NULL,
        /* [291] = */ NULL,
        /* [292] = */ NULL,
        /* [293] = */ NULL,
        /* [294] = */ NULL,
        /* [295] = */ NULL,
        /* [296] = */ NULL,
        /* [297] = */ NULL,
        /* [298] = */ NULL,
        /* [299] = */ NULL,
        /* [300] = */ NULL,
        /* [301] = */ NULL,
        /* [302] = */ NULL,
        /* [303] = */ NULL,
        /* [304] = */ NULL,
        /* [305] = */ NULL,
        /* [306] = */ NULL,
        /* [307] = */ NULL,
        /* [308] = */ NULL,
        /* [309] = */ NULL,
        /* [310] = */ NULL,
        /* [311] = */ NULL,
        /* [312] = */ NULL,
        /* [313] = */ NULL,
        /* [314] = */ NULL,
        /* [315] = */ NULL,
        /* [316] = */ NULL,
        /* [317] = */ NULL,
        /* [318] = */ NULL,
        /* [319] = */ NULL,
        /* [320] = */ NULL,
        /* [321] = */ NULL,
        /* [322] = */ NULL,
        /* [323] = */ NULL,
        /* [324] = */ NULL,
        /* [325] = */ NULL,
        /* [326] = */ NULL,
        /* [327] = */ NULL,
        /* [328] = */ NULL,
        /* [329] = */ NULL,
        /* [330] = */ NULL,
        /* [331] = */ NULL,
        /* [332] = */ NULL,
        /* [333] = */ NULL,
        /* [334] = */ NULL,
        /* [335] = */ NULL,
        /* [336] = */ NULL,
        /* [337] = */ NULL,
        /* [338] = */ NULL,
        /* [339] = */ NULL,
        /* [340] = */ NULL,
        /* [341] = */ NULL,
        /* [342] = */ NULL,
        /* [343] = */ NULL,
        /* [344] = */ NULL,
        /* [345] = */ NULL,
        /* [346] = */ NULL,
        /* [347] = */ NULL,
        /* [348] = */ NULL,
        /* [349] = */ NULL,
        /* [350] = */ NULL,
        /* [351] = */ NULL,
        /* [352] = */ NULL,
        /* [353] = */ NULL,
        /* [354] = */ NULL,
        /* [355] = */ NULL,
        /* [356] = */ NULL,
        /* [357] = */ NULL,
        /* [358] = */ NULL,
        /* [359] = */ NULL,
        /* [360] = */ NULL,
        /* [361] = */ NULL,
        /* [362] = */ NULL,
        /* [363] = */ NULL,
        /* [364] = */ NULL,
        /* [365] = */ NULL,
        /* [366] = */ NULL,
        /* [367] = */ NULL,
        /* [368] = */ NULL,
        /* [369] = */ NULL,
        /* [370] = */ NULL,
        /* [371] = */ NULL,
        /* [372] = */ NULL,
        /* [373] = */ NULL,
        /* [374] = */ NULL,
        /* [375] = */ NULL,
        /* [376] = */ NULL,
        /* [377] = */ NULL,
        /* [378] = */ NULL,
        /* [379] = */ NULL,
        /* [380] = */ NULL,
        /* [381] = */ NULL,
        /* [382] = */ NULL,
        /* [383] = */ NULL,
        /* [384] = */ NULL,
        /* [385] = */ NULL,
        /* [386] = */ NULL,
        /* [387] = */ NULL,
        /* [388] = */ NULL,
        /* [389] = */ NULL,
        /* [390] = */ NULL,
        /* [391] = */ NULL,
        /* [392] = */ NULL,
        /* [393] = */ NULL,
        /* [394] = */ NULL,
        /* [395] = */ NULL,
        /* [396] = */ NULL,
        /* [397] = */ NULL,
        /* [398] = */ NULL,
        /* [399] = */ NULL,
        /* [400] = */ NULL,
        /* [401] = */ NULL,
        /* [402] = */ NULL,
        /* [403] = */ NULL,
        /* [404] = */ NULL,
        /* [405] = */ NULL,
        /* [406] = */ NULL,
        /* [407] = */ NULL,
        /* [408] = */ NULL,
        /* [409] = */ NULL,
        /* [410] = */ NULL,
        /* [411] = */ NULL,
        /* [412] = */ NULL,
        /* [413] = */ NULL,
        /* [414] = */ NULL,
        /* [415] = */ NULL,
        /* [416] = */ NULL,
        /* [417] = */ NULL,
        /* [418] = */ NULL,
        /* [419] = */ NULL,
        /* [420] = */ NULL,
        /* [421] = */ NULL,
        /* [422] = */ NULL,
        /* [423] = */ NULL,
        /* [424] = */ NULL,
        /* [425] = */ NULL,
        /* [426] = */ NULL,
        /* [427] = */ NULL,
        /* [428] = */ NULL,
        /* [429] = */ NULL,
        /* [430] = */ NULL,
        /* [431] = */ NULL,
        /* [432] = */ NULL,
        /* [433] = */ NULL,
        /* [434] = */ NULL,
        /* [435] = */ NULL,
        /* [436] = */ NULL,
        /* [437] = */ NULL,
        /* [438] = */ NULL,
        /* [439] = */ NULL,
        /* [440] = */ NULL,
        /* [441] = */ NULL,
        /* [442] = */ NULL,
        /* [443] = */ NULL,
        /* [444] = */ NULL,
        /* [445] = */ NULL,
        /* [446] = */ NULL,
        /* [447] = */ NULL,
        /* [448] = */ NULL,
        /* [449] = */ NULL,
        /* [450] = */ NULL,
        /* [451] = */ NULL,
        /* [452] = */ NULL,
        /* [453] = */ NULL,
        /* [454] = */ NULL,
        /* [455] = */ NULL,
        /* [456] = */ NULL,
        /* [457] = */ NULL,
        /* [458] = */ NULL,
        /* [459] = */ NULL,
        /* [460] = */ NULL,
        /* [461] = */ NULL,
        /* [462] = */ NULL,
        /* [463] = */ NULL,
        /* [464] = */ NULL,
        /* [465] = */ NULL,
        /* [466] = */ NULL,
        /* [467] = */ NULL,
        /* [468] = */ NULL,
        /* [469] = */ NULL,
        /* [470] = */ NULL,
        /* [471] = */ NULL,
        /* [472] = */ NULL,
        /* [473] = */ NULL,
        /* [474] = */ NULL,
        /* [475] = */ NULL,
        /* [476] = */ NULL,
        /* [477] = */ NULL,
        /* [478] = */ NULL,
        /* [479] = */ NULL,
        /* [480] = */ NULL,
        /* [481] = */ NULL,
        /* [482] = */ NULL,
        /* [483] = */ NULL,
        /* [484] = */ NULL,
        /* [485] = */ NULL,
        /* [486] = */ NULL,
        /* [487] = */ NULL,
        /* [488] = */ NULL,
        /* [489] = */ NULL,
        /* [490] = */ NULL,
        /* [491] = */ NULL,
        /* [492] = */ NULL,
        /* [493] = */ NULL,
        /* [494] = */ NULL,
        /* [495] = */ NULL,
        /* [496] = */ NULL,
        /* [497] = */ NULL,
        /* [498] = */ NULL,
        /* [499] = */ NULL,
        /* [500] = */ NULL,
        /* [501] = */ NULL,
        /* [502] = */ NULL,
        /* [503] = */ NULL,
        /* [504] = */ NULL,
        /* [505] = */ NULL,
        /* [506] = */ NULL,
        /* [507] = */ NULL,
        /* [508] = */ NULL,
        /* [509] = */ NULL,
        /* [510] = */ NULL,
        /* [511] = */ NULL,
        /* [512] = */ "ERESTARTSYS",
        /* [513] = */ "ERESTARTNOINTR",
        /* [514] = */ "ERESTARTNOHAND",
        /* [515] = */ "ENOIOCTLCMD",
        /* [516] = */ "ERESTART_RESTARTBLOCK",
        /* [517] = */ "EPROBE_DEFER",
        /* [518] = */ "EOPENSTALE",
        /* [519] = */ NULL,
        /* [520] = */ NULL,
        /* [521] = */ "EBADHANDLE",
        /* [522] = */ "ENOTSYNC",
        /* [523] = */ "EBADCOOKIE",
        /* [524] = */ "ENOTSUPP",
        /* [525] = */ "ETOOSMALL",
        /* [526] = */ "ESERVERFAULT",
        /* [527] = */ "EBADTYPE",
        /* [528] = */ "EJUKEBOX",
        /* [529] = */ "EIOCBQUEUED",
        /* [530] = */ "ERECALLCONFLICT",
};

static inline std::string
errno_name(int errno_value) {

    const auto name = errno_names.at(errno_value);

    if(!name) {
        return "EUNKNOWN";
    }

    return name;
}

static inline std::string
errno_message(int errno_value) {
    // 1024 should be more than enough for most locales
    constexpr const std::size_t MAX_ERROR_MSG = 0x400;
    std::array<char, MAX_ERROR_MSG> errstr;
    char* msg = ::strerror_r(errno_value, errstr.data(), MAX_ERROR_MSG);
    return std::string{msg};
}

} // namespace gkfs::syscall

#endif // GKFS_SYSCALLS_ERRNO_HPP
