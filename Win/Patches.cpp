// Patches.cpp - Patches to fix problems in specific Infocom games.
// Copyright (c) 2017-2025 Chris Spiegel.
//
// This file is based on code taken from the Bocfel interpreter,
// written by Chris Spiegel. It is released under the MIT licence.

#include "stdafx.h"

#include <string>
#include <vector>

extern "C"
{
typedef int f_bool;
#define bool f_bool
#include "frotz.h"
#undef bool
}

struct Replacement
{
    uint32_t addr;
    uint32_t n;
    std::vector<zbyte> in;
    std::vector<zbyte> out;
};

struct Patch
{
    enum story story_id;
    const char (&serial)[sizeof h_serial + 1];
    zword release;
    zword checksum;
    std::vector<Replacement> replacements;
};

static std::vector<Patch> patches =
{
    // In Arthur, there is a routine called INIT-STATUS-LINE which does this:
    //
    // <PUTB ,K-DIROUT-TBL 0 !\ >
    // <SET N </ <WINGET 1 ,K-W-XSIZE> ,GL-SPACE-WIDTH>>
    // <COPYT ,K-DIROUT-TBL <ZREST ,K-DIROUT-TBL 1> <- .N>>
    //
    // The intent is to fill K-DIROUT-TBL with as many spaces as there
    // are characters in a line, but K-DIROUT-TBL is only 255 bytes
    // wide. If K-W-XSIZE / GL-SPACE-WIDTH is greater than 255, it will
    // overrun the buffer. This patch pulls the width from the value at
    // 0x21 in the header, which is the width of the screen in
    // characters, capped at 255.
    {
        ARTHUR, "890606", 54, 0x8e4a,
        {
            {
                0x9e29, 10,
                {0xbe, 0x13, 0x5f, 0x01, 0x03, 0x00, 0x77, 0x00, 0x1f, 0x01},
                {0x10, 0x00, 0x21, 0x01, 0xb4, 0xb4, 0xb4, 0xb4, 0xb4, 0xb4},
            }
        }
    },

    {
        ARTHUR, "890622", 63, 0x45eb,
        {
            {
                0x9e31, 10,
                {0xbe, 0x13, 0x5f, 0x01, 0x03, 0x00, 0x77, 0x00, 0x1f, 0x01},
                {0x10, 0x00, 0x21, 0x01, 0xb4, 0xb4, 0xb4, 0xb4, 0xb4, 0xb4}
            }
        }
    },

    {
        ARTHUR, "890714", 74, 0xd526,
        {
            {
                0x9de8, 10,
                {0xbe, 0x13, 0x5f, 0x01, 0x03, 0x00, 0x77, 0x00, 0x1f, 0x01},
                {0x10, 0x00, 0x21, 0x01, 0xb4, 0xb4, 0xb4, 0xb4, 0xb4, 0xb4}
            }
        }
    },

    // There are several patches for Beyond Zork.
    //
    // In four places it tries to treat a dictionary word as an object.
    // The offending code looks like:
    //
    // <REPLACE-SYN? ,W?CIRCLET ,W?FILM ,W?ZZZP>
    // <REPLACE-ADJ? ,W?CIRCLET ,W?SWIRLING ,W?ZZZP>
    //
    // Here ",W?CIRCLET" is the dictionary word "circlet"; what was
    // intended is ",CIRCLET", the object. Given that the object number
    // of the circlet is easily discoverable, these calls can be
    // rewritten to use the object number instead of the incorrect
    // dictionary entry.
    //
    // Beyond Zork also uses platform-specific hacks to display
    // character graphics.
    //
    // On IBM PC, if the interpreter clears the pictures bit,
    // Beyond Zork assumes code page 437 is active, and it uses arrows
    // and box-drawing characters which are invalid or gibberish in
    // ZSCII.
    //
    // On Apple IIc, in much the same way, Beyond Zork prints out values
    // which are valid MouseText, but are gibberish in ZSCII.
    //
    // To work around this, replace invalid characters with valid
    // characters. Beyond Zork uses font 1 with IBM and font 3 with
    // Apple, so while the IBM patches have to use a crude conversion
    // (e.g. "U" and "D" for up and down arrows, and "+" for all
    // box-drawing corners), the Apple patches convert to appropriate
    // font 3 values.
    {
        BEYOND_ZORK, "870915", 47, 0x3ff4,
        {
            // Circlet object
            { 0x2f8e6, 2, {0xa3, 0x9a}, {0x01, 0x86} },
            { 0x2f8f0, 2, {0xa3, 0x9a}, {0x01, 0x86} },
            { 0x2f902, 2, {0xa3, 0x9a}, {0x01, 0x86} },
            { 0x2f90c, 2, {0xa3, 0x9a}, {0x01, 0x86} },

            // IBM characters
            { 0xe58f, 1, {0xda}, {0x2b} },
            { 0xe595, 1, {0xc4}, {0x2d} },
            { 0xe59d, 1, {0xbf}, {0x2b} },
            { 0xe5a6, 1, {0xc0}, {0x2b} },
            { 0xe5ac, 1, {0xc4}, {0x2d} },
            { 0xe5b4, 1, {0xd9}, {0x2b} },
            { 0xe5c0, 1, {0xb3}, {0x7c} },
            { 0xe5c9, 1, {0xb3}, {0x7c} },
            { 0x3b211, 1, {0x18}, {0x55} },
            { 0x3b244, 1, {0x19}, {0x44} },
            { 0x3b340, 1, {0x1b}, {0x4c} },
            { 0x3b37f, 1, {0x1a}, {0x52} },

            // Apple IIc characters
            { 0xe5ee, 1, {0x4c}, {0x4b} },
            { 0xe5fd, 1, {0x5a}, {0x4e} },
            { 0xe604, 1, {0x5f}, {0x4d} },
            { 0xe86a, 1, {0x5a}, {0x4e} },
            { 0xe8a6, 1, {0x5f}, {0x4d} },
            { 0x3b220, 1, {0x4b}, {0x5c} },
            { 0x3b253, 1, {0x4a}, {0x5d} },
            { 0x3b34f, 1, {0x48}, {0x21} },
            { 0x3b38e, 1, {0x55}, {0x22} },
        },
    },

    {
        BEYOND_ZORK, "870917", 49, 0x24d6,
        {
            // Circlet object
            { 0x2f8b6, 2, {0xa3, 0x9c}, {0x01, 0x86} },
            { 0x2f8c0, 2, {0xa3, 0x9c}, {0x01, 0x86} },
            { 0x2f8d2, 2, {0xa3, 0x9c}, {0x01, 0x86} },
            { 0x2f8dc, 2, {0xa3, 0x9c}, {0x01, 0x86} },

            // IBM characters
            { 0xe4c7, 1, {0xda}, {0x2b} },
            { 0xe4cd, 1, {0xc4}, {0x2d} },
            { 0xe4d5, 1, {0xbf}, {0x2b} },
            { 0xe4de, 1, {0xc0}, {0x2b} },
            { 0xe4e4, 1, {0xc4}, {0x2d} },
            { 0xe4ec, 1, {0xd9}, {0x2b} },
            { 0xe4f8, 1, {0xb3}, {0x7c} },
            { 0xe501, 1, {0xb3}, {0x7c} },
            { 0x3b1e1, 1, {0x18}, {0x55} },
            { 0x3b214, 1, {0x19}, {0x44} },
            { 0x3b310, 1, {0x1b}, {0x4c} },
            { 0x3b34f, 1, {0x1a}, {0x52} },

            // Apple IIc characters
            { 0xe526, 1, {0x4c}, {0x4b} },
            { 0xe535, 1, {0x5a}, {0x4e} },
            { 0xe53c, 1, {0x5f}, {0x4d} },
            { 0xe7a2, 1, {0x5a}, {0x4e} },
            { 0xe7de, 1, {0x5f}, {0x4d} },
            { 0x3b1f0, 1, {0x4b}, {0x5c} },
            { 0x3b223, 1, {0x4a}, {0x5d} },
            { 0x3b31f, 1, {0x48}, {0x21} },
            { 0x3b35e, 1, {0x55}, {0x22} },
        },
    },

    {
        BEYOND_ZORK, "870923", 51, 0x0cbe,
        {
            // Circlet object
            { 0x2f762, 2, {0xa3, 0x8d}, {0x01, 0x86} },
            { 0x2f76c, 2, {0xa3, 0x8d}, {0x01, 0x86} },
            { 0x2f77e, 2, {0xa3, 0x8d}, {0x01, 0x86} },
            { 0x2f788, 2, {0xa3, 0x8d}, {0x01, 0x86} },

            // IBM characters
            { 0xe4af, 1, {0xda}, {0x2b} },
            { 0xe4b5, 1, {0xc4}, {0x2d} },
            { 0xe4bd, 1, {0xbf}, {0x2b} },
            { 0xe4c6, 1, {0xc0}, {0x2b} },
            { 0xe4cc, 1, {0xc4}, {0x2d} },
            { 0xe4d4, 1, {0xd9}, {0x2b} },
            { 0xe4e0, 1, {0xb3}, {0x7c} },
            { 0xe4e9, 1, {0xb3}, {0x7c} },
            { 0x3b08d, 1, {0x18}, {0x55} },
            { 0x3b0c0, 1, {0x19}, {0x44} },
            { 0x3b1bc, 1, {0x1b}, {0x4c} },
            { 0x3b1fb, 1, {0x1a}, {0x52} },

            // Apple IIc characters
            { 0xe50e, 1, {0x4c}, {0x4b} },
            { 0xe51d, 1, {0x5a}, {0x4e} },
            { 0xe524, 1, {0x5f}, {0x4d} },
            { 0xe78a, 1, {0x5a}, {0x4e} },
            { 0xe7c6, 1, {0x5f}, {0x4d} },
            { 0x3b09c, 1, {0x4b}, {0x5c} },
            { 0x3b0cf, 1, {0x4a}, {0x5d} },
            { 0x3b1cb, 1, {0x48}, {0x21} },
            { 0x3b20a, 1, {0x55}, {0x22} },
        },
    },

    {
        BEYOND_ZORK, "871221", 57, 0xc5ad,
        {
            // Circlet object
            { 0x2fc72, 2, {0xa3, 0xba}, {0x01, 0x87} },
            { 0x2fc7c, 2, {0xa3, 0xba}, {0x01, 0x87} },
            { 0x2fc8e, 2, {0xa3, 0xba}, {0x01, 0x87} },
            { 0x2fc98, 2, {0xa3, 0xba}, {0x01, 0x87} },

            // IBM characters
            { 0xe58b, 1, {0xda}, {0x2b} },
            { 0xe591, 1, {0xc4}, {0x2d} },
            { 0xe599, 1, {0xbf}, {0x2b} },
            { 0xe5a2, 1, {0xc0}, {0x2b} },
            { 0xe5a8, 1, {0xc4}, {0x2d} },
            { 0xe5b0, 1, {0xd9}, {0x2b} },
            { 0xe5bc, 1, {0xb3}, {0x7c} },
            { 0xe5c5, 1, {0xb3}, {0x7c} },
            { 0x3b1dd, 1, {0x18}, {0x55} },
            { 0x3b210, 1, {0x19}, {0x44} },
            { 0x3b30c, 1, {0x1b}, {0x4c} },
            { 0x3b34b, 1, {0x1a}, {0x52} },

            // Apple IIc characters
            { 0xe5ea, 1, {0x4c}, {0x4b} },
            { 0xe5f9, 1, {0x5a}, {0x4e} },
            { 0xe600, 1, {0x5f}, {0x4d} },
            { 0xe866, 1, {0x5a}, {0x4e} },
            { 0xe8a2, 1, {0x5f}, {0x4d} },
            { 0x3b1ec, 1, {0x4b}, {0x5c} },
            { 0x3b21f, 1, {0x4a}, {0x5d} },
            { 0x3b31b, 1, {0x48}, {0x21} },
            { 0x3b35a, 1, {0x55}, {0x22} },
        },
    },

    {
        BEYOND_ZORK, "880610", 60, 0xa49d,
        {
            // Circlet object
            { 0x2fbfe, 2, {0xa3, 0xc0}, {0x01, 0x87} },
            { 0x2fc08, 2, {0xa3, 0xc0}, {0x01, 0x87} },
            { 0x2fc1a, 2, {0xa3, 0xc0}, {0x01, 0x87} },
            { 0x2fc24, 2, {0xa3, 0xc0}, {0x01, 0x87} },

            // IBM characters
            { 0xe4e3, 1, {0xda}, {0x2b} },
            { 0xe4e9, 1, {0xc4}, {0x2d} },
            { 0xe4f1, 1, {0xbf}, {0x2b} },
            { 0xe4fa, 1, {0xc0}, {0x2b} },
            { 0xe500, 1, {0xc4}, {0x2d} },
            { 0xe508, 1, {0xd9}, {0x2b} },
            { 0xe514, 1, {0xb3}, {0x7c} },
            { 0xe51d, 1, {0xb3}, {0x7c} },
            { 0x3b169, 1, {0x18}, {0x55} },
            { 0x3b19c, 1, {0x19}, {0x44} },
            { 0x3b298, 1, {0x1b}, {0x4c} },
            { 0x3b2d7, 1, {0x1a}, {0x52} },

            // Apple IIc characters
            { 0xe542, 1, {0x4c}, {0x4b} },
            { 0xe551, 1, {0x5a}, {0x4e} },
            { 0xe558, 1, {0x5f}, {0x4d} },
            { 0xe7be, 1, {0x5a}, {0x4e} },
            { 0xe7fa, 1, {0x5f}, {0x4d} },
            { 0x3b178, 1, {0x4b}, {0x5c} },
            { 0x3b1ab, 1, {0x4a}, {0x5d} },
            { 0x3b2a7, 1, {0x48}, {0x21} },
            { 0x3b2e6, 1, {0x55}, {0x22} },
        },
    },

    // Bureaucracy has a routine meant to sleep for a certain amount of
    // time, but it implements this using a busy loop based on the
    // target machine, each machine having its own iteration count which
    // should approximate the number of seconds passed in. This doesn't
    // work on modern machines at all, as they're far too fast. A
    // replacement is provided here which does the following:
    //
    // [ Delay n;
    //     @mul n $0a -> n;
    //     @read_char 1 n Pause -> sp;
    //     @rtrue;
    // ];
    //
    // [ Pause;
    //     @rtrue;
    // ];
    {
        BUREAUCRACY, "870212", 86, 0xe024,
        {
            {
                0x2128c, 18,
                {0x02, 0x00, 0x01, 0x00, 0x00, 0x10, 0x00, 0x1e, 0x00, 0xd0, 0x2f, 0xde, 0x5c, 0x00, 0x02, 0xd6, 0x2f, 0x03},
                {0x01, 0x00, 0x01, 0x56, 0x01, 0x0a, 0x01, 0xf6, 0x63, 0x01, 0x01, 0x84, 0xa7, 0x00, 0xb0, 0x00, 0x00, 0xb0},
            }
        }
    },
    {
        BUREAUCRACY, "870602", 116, 0xfc65,
        {
            {
                0x212d0, 18,
                {0x02, 0x00, 0x01, 0x00, 0x00, 0x10, 0x00, 0x1e, 0x00, 0xd0, 0x2f, 0xde, 0x64, 0x00, 0x02, 0xd6, 0x2f, 0x03},
                {0x01, 0x00, 0x01, 0x56, 0x01, 0x0a, 0x01, 0xf6, 0x63, 0x01, 0x01, 0x84, 0xb8, 0x00, 0xb0, 0x00, 0x00, 0xb0},
            }
        }
    },
    {
        BUREAUCRACY, "880521", 160, 0x07f0,
        {
            {
                0x212c8, 18,
                {0x02, 0x00, 0x01, 0x00, 0x00, 0x10, 0x00, 0x1e, 0x00, 0xd0, 0x2f, 0xdc, 0xce, 0x00, 0x02, 0xd6, 0x2f, 0x03},
                {0x01, 0x00, 0x01, 0x56, 0x01, 0x0a, 0x01, 0xf6, 0x63, 0x01, 0x01, 0x84, 0xb6, 0x00, 0xb0, 0x00, 0x00, 0xb0},
            }
        }
    },

    // Journey doubles up the word "essence" when examining Praxix's
    // pouch: originally the game printed short descriptions when the
    // screen was narrow enough, which didn't include the word
    // "essence", so it always just printed out an extra "essence",
    // knowing that it wasn't shown. However, release 51 changed this so
    // that the long descriptions, which included "essence", were
    // unconditionally printed in this circumstance; but the extra
    // "essence" remained. This patches out the printing of that extra
    // word by replacing it with @nop.
    {
        JOURNEY, "890522", 51, 0x4f59,
        {{ 0x9d9f, 3, {0xb2, 0x80, 0x3e}, {0xb4, 0xb4, 0xb4} }},
    },
    {
        JOURNEY, "890526", 54, 0x5707,
        {{ 0x9e2f, 3, {0xb2, 0x80, 0x3e}, {0xb4, 0xb4, 0xb4} }},
    },
    {
        JOURNEY, "890616", 77, 0xb136,
        {{ 0xa09f, 3, {0xb2, 0x80, 0x3e}, {0xb4, 0xb4, 0xb4} }},
    },
    {
        JOURNEY, "890627", 79, 0xff04,
        {{ 0xa10b, 3, {0xb2, 0x80, 0x3e}, {0xb4, 0xb4, 0xb4} }},
    },
    {
        JOURNEY, "890706", 83, 0xd2b8,
        {{ 0xa0ef, 3, {0xb2, 0x80, 0x3e}, {0xb4, 0xb4, 0xb4} }},
    },

    // Journey has a CHANGE-FONT routine it uses to change the font, but
    // it only supports fonts 3 and 4, so when it wants to set the font
    // back to 1, nothing happens. This is worked around here by,
    // instead of executing RFALSE when the font is unsupported, jumping
    // to the <FONT 1> call at the end of the routine.
    //
    // This fix was created by Petter Sjölund.

    {
        JOURNEY, "890522", 51, 0x4f59,
        {{ 0x471f, 1, {0x40}, {0x4b} }},
    },
    {
        JOURNEY, "890526", 54, 0x5707,
        {{ 0x4727, 1, {0x40}, {0x4b} }},
    },
    {
        JOURNEY, "890616", 77, 0xb136,
        {{ 0x474f, 1, {0x40}, {0x4b} }},
    },
    {
        JOURNEY, "890627", 79, 0xff04,
        {{ 0x4757, 1, {0x40}, {0x4b} }},
    },
    {
        JOURNEY, "890706", 83, 0xd2b8,
        {{ 0x4757, 1, {0x40}, {0x4b} }},
    },

    // This is in a routine which iterates over all attributes of an
    // object, but due to an off-by-one error, attribute 48 (0x30) is
    // included, which is not valid, as the last attribute is 47 (0x2f);
    // there are 48 attributes but the count starts at 0.
    {
        SHERLOCK, "871214", 21, 0x79b2,
        {{ 0x223ac, 1, {0x30}, {0x2f} }},
    },
    {
        SHERLOCK, "880112", 22, 0xcb96,
        {{ 0x225a4, 1, {0x30}, {0x2f} }},
    },
    {
        SHERLOCK, "880127", 26, 0x26ba,
        {{ 0x22818, 1, {0x30}, {0x2f} }},
    },
    {
        SHERLOCK, "880324", 4, 0x7086,
        {{ 0x22498, 1, {0x30}, {0x2f} }},
    },

    // The operands to @get_prop here are swapped, so swap them back.
    {
        STATIONFALL, "870326", 87, 0x71ae,
        {{ 0xd3d4, 3, {0x31, 0x0c, 0x73}, {0x51, 0x73, 0x0c} }},
    },
    {
        STATIONFALL, "870430", 107, 0x2871,
        {{ 0xe3fe, 3, {0x31, 0x0c, 0x77}, {0x51, 0x77, 0x0c} }},
    },

    // The Solid Gold (V5) version of Wishbringer calls @show_status, but
    // that is an illegal instruction outside of V3. Convert it to @nop.
    {
        WISHBRINGER, "880706", 23, 0x4222,
        {{ 0x1f910, 1, {0xbc}, {0xb4} }},
    }
};

static bool apply_patch(const Replacement &r)
{
    if (r.addr >= h_dynamic_size &&
        r.addr + r.n < (uint32_t)story_size &&
        std::memcmp(&zmp[r.addr], r.in.data(), r.n) == 0) {

        std::memcpy(&zmp[r.addr], r.out.data(), r.n);

        return true;
    }

    return false;
}

static bool sanity_check(const Patch &patch)
{
    bool ok = true;

    for (const auto &replacement : patch.replacements) {
        if (replacement.in.size() != replacement.n ||
            replacement.out.size() != replacement.n)
        {
            ok = false;
        }
    }

    return ok;
}

static void apply_patches(const std::vector<Patch> &patches)
{
    for (const auto &patch : patches) {
        if (sanity_check(patch) &&
            patch.story_id == story_id &&
            std::memcmp(patch.serial, h_serial, sizeof h_serial) == 0 &&
            patch.release == h_release &&
            patch.checksum == h_checksum) {

            for (const auto &replacement : patch.replacements) {
                apply_patch(replacement);
            }
        }
    }
}

void apply_patches()
{
    apply_patches(patches);
}
