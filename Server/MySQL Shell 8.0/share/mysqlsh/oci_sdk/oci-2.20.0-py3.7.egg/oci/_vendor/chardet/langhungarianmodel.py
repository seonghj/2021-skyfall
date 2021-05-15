# coding: utf-8
# Modified Work: Copyright (c) 2018, 2020, Oracle and/or its affiliates.  All rights reserved.
# This software is dual-licensed to you under the Universal Permissive License (UPL) 1.0 as shown at https://oss.oracle.com/licenses/upl or Apache License 2.0 as shown at http://www.apache.org/licenses/LICENSE-2.0. You may choose either license.
# Original Work: Copyright (c) 2018 Character Encoding Detector contributors.  https://github.com/chardet

# 255: Control characters that usually does not exist in any text
# 254: Carriage/Return
# 253: symbol (punctuation) that does not belong to word
# 252: 0 - 9

# Character Mapping Table:
Latin2_HungarianCharToOrderMap = (
255,255,255,255,255,255,255,255,255,255,254,255,255,254,255,255,  # 00
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  # 10
253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,  # 20
252,252,252,252,252,252,252,252,252,252,253,253,253,253,253,253,  # 30
253, 28, 40, 54, 45, 32, 50, 49, 38, 39, 53, 36, 41, 34, 35, 47,
 46, 71, 43, 33, 37, 57, 48, 64, 68, 55, 52,253,253,253,253,253,
253,  2, 18, 26, 17,  1, 27, 12, 20,  9, 22,  7,  6, 13,  4,  8,
 23, 67, 10,  5,  3, 21, 19, 65, 62, 16, 11,253,253,253,253,253,
159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,
175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,
191,192,193,194,195,196,197, 75,198,199,200,201,202,203,204,205,
 79,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,
221, 51, 81,222, 78,223,224,225,226, 44,227,228,229, 61,230,231,
232,233,234, 58,235, 66, 59,236,237,238, 60, 69, 63,239,240,241,
 82, 14, 74,242, 70, 80,243, 72,244, 15, 83, 77, 84, 30, 76, 85,
245,246,247, 25, 73, 42, 24,248,249,250, 31, 56, 29,251,252,253,
)

win1250HungarianCharToOrderMap = (
255,255,255,255,255,255,255,255,255,255,254,255,255,254,255,255,  # 00
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  # 10
253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,  # 20
252,252,252,252,252,252,252,252,252,252,253,253,253,253,253,253,  # 30
253, 28, 40, 54, 45, 32, 50, 49, 38, 39, 53, 36, 41, 34, 35, 47,
 46, 72, 43, 33, 37, 57, 48, 64, 68, 55, 52,253,253,253,253,253,
253,  2, 18, 26, 17,  1, 27, 12, 20,  9, 22,  7,  6, 13,  4,  8,
 23, 67, 10,  5,  3, 21, 19, 65, 62, 16, 11,253,253,253,253,253,
161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,
177,178,179,180, 78,181, 69,182,183,184,185,186,187,188,189,190,
191,192,193,194,195,196,197, 76,198,199,200,201,202,203,204,205,
 81,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,
221, 51, 83,222, 80,223,224,225,226, 44,227,228,229, 61,230,231,
232,233,234, 58,235, 66, 59,236,237,238, 60, 70, 63,239,240,241,
 84, 14, 75,242, 71, 82,243, 73,244, 15, 85, 79, 86, 30, 77, 87,
245,246,247, 25, 74, 42, 24,248,249,250, 31, 56, 29,251,252,253,
)

# Model Table:
# total sequences: 100%
# first 512 sequences: 94.7368%
# first 1024 sequences:5.2623%
# rest  sequences:     0.8894%
# negative sequences:  0.0009%
HungarianLangModel = (
0,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
3,3,3,3,3,3,3,3,3,3,2,3,3,3,3,3,3,3,3,2,2,3,3,1,1,2,2,2,2,2,1,2,
3,2,2,3,3,3,3,3,2,3,3,3,3,3,3,1,2,3,3,3,3,2,3,3,1,1,3,3,0,1,1,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,
3,2,1,3,3,3,3,3,2,3,3,3,3,3,1,1,2,3,3,3,3,3,3,3,1,1,3,2,0,1,1,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,
3,3,3,3,3,3,3,3,3,3,3,1,1,2,3,3,3,1,3,3,3,3,3,1,3,3,2,2,0,3,2,3,
0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,
3,3,3,3,3,3,2,3,3,3,2,3,3,2,3,3,3,3,3,2,3,3,2,2,3,2,3,2,0,3,2,2,
0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,
3,3,3,3,3,3,2,3,3,3,3,3,2,3,3,3,1,2,3,2,2,3,1,2,3,3,2,2,0,3,3,3,
0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
3,3,3,3,3,3,3,3,3,3,2,2,3,3,3,3,3,3,2,3,3,3,3,2,3,3,3,3,0,2,3,2,
0,0,0,1,1,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
3,3,3,3,3,3,3,3,3,3,3,1,1,1,3,3,2,1,3,2,2,3,2,1,3,2,2,1,0,3,3,1,
0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
3,2,2,3,3,3,3,3,1,2,3,3,3,3,1,2,1,3,3,3,3,2,2,3,1,1,3,2,0,1,1,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,
3,3,3,3,3,3,3,3,2,2,3,3,3,3,3,2,1,3,3,3,3,3,2,2,1,3,3,3,0,1,1,2,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,
3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,3,3,3,2,3,3,2,3,3,3,2,0,3,2,3,
0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,1,0,
3,3,3,3,3,3,2,3,3,3,2,3,2,3,3,3,1,3,2,2,2,3,1,1,3,3,1,1,0,3,3,2,
0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
3,3,3,3,3,3,3,2,3,3,3,2,3,2,3,3,3,2,3,3,3,3,3,1,2,3,2,2,0,2,2,2,
0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
3,3,3,2,2,2,3,1,3,3,2,2,1,3,3,3,1,1,3,1,2,3,2,3,2,2,2,1,0,2,2,2,
0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,
3,1,1,3,3,3,3,3,1,2,3,3,3,3,1,2,1,3,3,3,2,2,3,2,1,0,3,2,0,1,1,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
3,1,1,3,3,3,3,3,1,2,3,3,3,3,1,1,0,3,3,3,3,0,2,3,0,0,2,1,0,1,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
3,3,3,3,3,3,2,2,3,3,2,2,2,2,3,3,0,1,2,3,2,3,2,2,3,2,1,2,0,2,2,2,
0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,
3,3,3,3,3,3,1,2,3,3,3,2,1,2,3,3,2,2,2,3,2,3,3,1,3,3,1,1,0,2,3,2,
0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
3,3,3,1,2,2,2,2,3,3,3,1,1,1,3,3,1,1,3,1,1,3,2,1,2,3,1,1,0,2,2,2,
0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
3,3,3,2,1,2,1,1,3,3,1,1,1,1,3,3,1,1,2,2,1,2,1,1,2,2,1,1,0,2,2,1,
0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
3,3,3,1,1,2,1,1,3,3,1,0,1,1,3,3,2,0,1,1,2,3,1,0,2,2,1,0,0,1,3,2,
0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
3,2,1,3,3,3,3,3,1,2,3,2,3,3,2,1,1,3,2,3,2,1,2,2,0,1,2,1,0,0,1,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,
3,3,3,3,2,2,2,2,3,1,2,2,1,1,3,3,0,3,2,1,2,3,2,1,3,3,1,1,0,2,1,3,
0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
3,3,3,2,2,2,3,2,3,3,3,2,1,1,3,3,1,1,1,2,2,3,2,3,2,2,2,1,0,2,2,1,
0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
1,0,0,3,3,3,3,3,0,0,3,3,2,3,0,0,0,2,3,3,1,0,1,2,0,0,1,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
3,1,2,3,3,3,3,3,1,2,3,3,2,2,1,1,0,3,3,2,2,1,2,2,1,0,2,2,0,1,1,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
3,3,2,2,1,3,1,2,3,3,2,2,1,1,2,2,1,1,1,1,3,2,1,1,1,1,2,1,0,1,2,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,
2,3,3,1,1,1,1,1,3,3,3,0,1,1,3,3,1,1,1,1,1,2,2,0,3,1,1,2,0,2,1,1,
0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
3,1,0,1,2,1,2,2,0,1,2,3,1,2,0,0,0,2,1,1,1,1,1,2,0,0,1,1,0,0,0,0,
1,2,1,2,2,2,1,2,1,2,0,2,0,2,2,1,1,2,1,1,2,1,1,1,0,1,0,0,0,1,1,0,
1,1,1,2,3,2,3,3,0,1,2,2,3,1,0,1,0,2,1,2,2,0,1,1,0,0,1,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0,0,3,3,2,2,1,0,0,3,2,3,2,0,0,0,1,1,3,0,0,1,1,0,0,2,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
3,1,1,2,2,3,3,1,0,1,3,2,3,1,1,1,0,1,1,1,1,1,3,1,0,0,2,2,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
3,1,1,1,2,2,2,1,0,1,2,3,3,2,0,0,0,2,1,1,1,2,1,1,1,0,1,1,1,0,0,0,
1,2,2,2,2,2,1,1,1,2,0,2,1,1,1,1,1,2,1,1,1,1,1,1,0,1,1,1,0,0,1,1,
3,2,2,1,0,0,1,1,2,2,0,3,0,1,2,1,1,0,0,1,1,1,0,1,1,1,1,0,2,1,1,1,
2,2,1,1,1,2,1,2,1,1,1,1,1,1,1,2,1,1,1,2,3,1,1,1,1,1,1,1,1,1,0,1,
2,3,3,0,1,0,0,0,3,3,1,0,0,1,2,2,1,0,0,0,0,2,0,0,1,1,1,0,2,1,1,1,
2,1,1,1,1,1,1,2,1,1,0,1,1,0,1,1,1,0,1,2,1,1,0,1,1,1,1,1,1,1,0,1,
2,3,3,0,1,0,0,0,2,2,0,0,0,0,1,2,2,0,0,0,0,1,0,0,1,1,0,0,2,0,1,0,
2,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,0,1,1,1,1,1,0,1,
3,2,2,0,1,0,1,0,2,3,2,0,0,1,2,2,1,0,0,1,1,1,0,0,2,1,0,1,2,2,1,1,
2,1,1,1,1,1,1,2,1,1,1,1,1,1,0,2,1,0,1,1,0,1,1,1,0,1,1,2,1,1,0,1,
2,2,2,0,0,1,0,0,2,2,1,1,0,0,2,1,1,0,0,0,1,2,0,0,2,1,0,0,2,1,1,1,
2,1,1,1,1,2,1,2,1,1,1,2,2,1,1,2,1,1,1,2,1,1,1,1,1,1,1,1,1,1,0,1,
1,2,3,0,0,0,1,0,3,2,1,0,0,1,2,1,1,0,0,0,0,2,1,0,1,1,0,0,2,1,2,1,
1,1,0,0,0,1,0,1,1,1,1,1,2,0,0,1,0,0,0,2,0,0,1,1,1,1,1,1,1,1,0,1,
3,0,0,2,1,2,2,1,0,0,2,1,2,2,0,0,0,2,1,1,1,0,1,1,0,0,1,1,2,0,0,0,
1,2,1,2,2,1,1,2,1,2,0,1,1,1,1,1,1,1,1,1,2,1,1,0,0,1,1,1,1,0,0,1,
1,3,2,0,0,0,1,0,2,2,2,0,0,0,2,2,1,0,0,0,0,3,1,1,1,1,0,0,2,1,1,1,
2,1,0,1,1,1,0,1,1,1,1,1,1,1,0,2,1,0,0,1,0,1,1,0,1,1,1,1,1,1,0,1,
2,3,2,0,0,0,1,0,2,2,0,0,0,0,2,1,1,0,0,0,0,2,1,0,1,1,0,0,2,1,1,0,
2,1,1,1,1,2,1,2,1,2,0,1,1,1,0,2,1,1,1,2,1,1,1,1,0,1,1,1,1,1,0,1,
3,1,1,2,2,2,3,2,1,1,2,2,1,1,0,1,0,2,2,1,1,1,1,1,0,0,1,1,0,1,1,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
2,2,2,0,0,0,0,0,2,2,0,0,0,0,2,2,1,0,0,0,1,1,0,0,1,2,0,0,2,1,1,1,
2,2,1,1,1,2,1,2,1,1,0,1,1,1,1,2,1,1,1,2,1,1,1,1,0,1,2,1,1,1,0,1,
1,0,0,1,2,3,2,1,0,0,2,0,1,1,0,0,0,1,1,1,1,0,1,1,0,0,1,0,0,0,0,0,
1,2,1,2,1,2,1,1,1,2,0,2,1,1,1,0,1,2,0,0,1,1,1,0,0,0,0,0,0,0,0,0,
2,3,2,0,0,0,0,0,1,1,2,1,0,0,1,1,1,0,0,0,0,2,0,0,1,1,0,0,2,1,1,1,
2,1,1,1,1,1,1,2,1,0,1,1,1,1,0,2,1,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,
1,2,2,0,1,1,1,0,2,2,2,0,0,0,3,2,1,0,0,0,1,1,0,0,1,1,0,1,1,1,0,0,
1,1,0,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,1,1,0,0,1,1,1,0,1,0,1,
2,1,0,2,1,1,2,2,1,1,2,1,1,1,0,0,0,1,1,0,1,1,1,1,0,0,1,1,1,0,0,0,
1,2,2,2,2,2,1,1,1,2,0,2,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,0,0,1,0,
1,2,3,0,0,0,1,0,2,2,0,0,0,0,2,2,0,0,0,0,0,1,0,0,1,0,0,0,2,0,1,0,
2,1,1,1,1,1,0,2,0,0,0,1,2,1,1,1,1,0,1,2,0,1,0,1,0,1,1,1,0,1,0,1,
2,2,2,0,0,0,1,0,2,1,2,0,0,0,1,1,2,0,0,0,0,1,0,0,1,1,0,0,2,1,0,1,
2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,0,1,1,1,1,1,0,1,
1,2,2,0,0,0,1,0,2,2,2,0,0,0,1,1,0,0,0,0,0,1,1,0,2,0,0,1,1,1,0,1,
1,0,1,1,1,1,1,1,0,1,1,1,1,0,0,1,0,0,1,1,0,1,0,1,1,1,1,1,0,0,0,1,
1,0,0,1,0,1,2,1,0,0,1,1,1,2,0,0,0,1,1,0,1,0,1,1,0,0,1,0,0,0,0,0,
0,2,1,2,1,1,1,1,1,2,0,2,0,1,1,0,1,2,1,0,1,1,1,0,0,0,0,0,0,1,0,0,
2,1,1,0,1,2,0,0,1,1,1,0,0,0,1,1,0,0,0,0,0,1,0,0,1,0,0,0,2,1,0,1,
2,2,1,1,1,1,1,2,1,1,0,1,1,1,1,2,1,1,1,2,1,1,0,1,0,1,1,1,1,1,0,1,
1,2,2,0,0,0,0,0,1,1,0,0,0,0,2,1,0,0,0,0,0,2,0,0,2,2,0,0,2,0,0,1,
2,1,1,1,1,1,1,1,0,1,1,0,1,1,0,1,0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,1,
1,1,2,0,0,3,1,0,2,1,1,1,0,0,1,1,1,0,0,0,1,1,0,0,0,1,0,0,1,0,1,0,
1,2,1,0,1,1,1,2,1,1,0,1,1,1,1,1,0,0,0,1,1,1,1,1,0,1,0,0,0,1,0,0,
2,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,1,0,0,0,1,0,0,0,0,2,0,0,0,
2,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,2,1,1,0,0,1,1,1,1,1,0,1,
2,1,1,1,2,1,1,1,0,1,1,2,1,0,0,0,0,1,1,1,1,0,1,0,0,0,0,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,1,0,1,1,1,1,1,0,0,1,1,2,1,0,0,0,1,1,0,0,0,1,1,0,0,1,0,1,0,0,0,
1,2,1,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,1,0,1,1,1,0,0,0,0,0,0,1,0,0,
2,0,0,0,1,1,1,1,0,0,1,1,0,0,0,0,0,1,1,1,2,0,0,1,0,0,1,0,1,0,0,0,
0,1,1,1,1,1,1,1,1,2,0,1,1,1,1,0,1,1,1,0,1,1,1,0,0,0,0,0,0,0,0,0,
1,0,0,1,1,1,1,1,0,0,2,1,0,1,0,0,0,1,0,1,0,0,0,0,0,0,1,0,0,0,0,0,
0,1,1,1,1,1,1,0,1,1,0,1,0,1,1,0,1,1,0,0,1,1,1,0,0,0,0,0,0,0,0,0,
1,0,0,1,1,1,0,0,0,0,1,0,2,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,
0,1,1,1,1,1,0,0,1,1,0,1,0,1,0,0,1,1,1,0,1,1,1,0,0,0,0,0,0,0,0,0,
0,0,0,1,0,0,0,0,0,0,1,1,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,1,1,1,0,1,0,0,1,1,0,1,0,1,1,0,1,1,1,0,1,1,1,0,0,0,0,0,0,0,0,0,
2,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,0,0,1,0,0,1,0,1,0,1,1,1,0,0,1,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0,0,1,1,1,1,0,0,0,1,1,1,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,
0,1,1,1,1,1,1,0,1,1,0,1,0,1,0,0,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,
)

Latin2HungarianModel = {
  'char_to_order_map': Latin2_HungarianCharToOrderMap,
  'precedence_matrix': HungarianLangModel,
  'typical_positive_ratio': 0.947368,
  'keep_english_letter': True,
  'charset_name': "ISO-8859-2",
  'language': 'Hungarian',
}

Win1250HungarianModel = {
  'char_to_order_map': win1250HungarianCharToOrderMap,
  'precedence_matrix': HungarianLangModel,
  'typical_positive_ratio': 0.947368,
  'keep_english_letter': True,
  'charset_name': "windows-1250",
  'language': 'Hungarian',
}
