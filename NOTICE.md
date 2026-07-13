This project is a C++ port of substantial portions of MonoGame.Extended.

MonoGame.Extended (https://github.com/craftworkgames/MonoGame.Extended) is licensed
under the MIT License.

MonoGame.Extended copyright: Dylan Wilson, Lucas Girouard-Stranks, Christopher Whitley,
and contributors (2015-2024). All individual source files in the upstream project carry
their own "Copyright (c) Craftwork Games" MIT header.

Full upstream license text:

> The MIT License (MIT)
>
> Copyright (c) 2015-2024:
> - Dylan Wilson (https://github.com/dylanwilson80)
> - Lucas Girouard-Stranks (https://github.com/lithiumtoast)
> - Christopher Whitley (https://github.com/aristurtledev)
>
> Permission is hereby granted, free of charge, to any person obtaining a copy
> of this software and associated documentation files (the "Software"), to deal
> in the Software without restriction, including without limitation the rights
> to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
> copies of the Software, and to permit persons to whom the Software is
> furnished to do so, subject to the following conditions:
>
> The above copyright notice and this permission notice shall be included in all
> copies or substantial portions of the Software.
>
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
> IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
> FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
> AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
> LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
> OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
> SOFTWARE.

Every file in this repository that was translated from a MonoGame.Extended source file
carries its own SPDX header pointing back to this notice; see `CLAUDE.md` for the exact
format.

## Code directly derived from other MIT-licensed projects

A small number of MonoGame.Extended source files carry their own upstream attribution
header crediting a project other than Craftwork Games as the origin of that specific
file's code — carried forward here since it applies to code actually ported into this
repository, not just "inspiration":

- **`Angle` (`include/CNA/Extended/Angle.hpp`, ported from MonoGame.Extended's
  `Math/Angle.cs`)** originates from the [SlimMath](http://code.google.com/p/slimmath/)
  project (greetings to the SlimDX Group, per MonoGame.Extended's own file header):
  > Copyright (c) 2007-2010 SlimDX Group
  >
  > Permission is hereby granted, free of charge, to any person obtaining a copy
  > of this software and associated documentation files (the "Software"), to deal
  > in the Software without restriction, including without limitation the rights
  > to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  > copies of the Software, and to permit persons to whom the Software is
  > furnished to do so, subject to the following conditions:
  >
  > The above copyright notice and this permission notice shall be included in
  > all copies or substantial portions of the Software.
  >
  > THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  > IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  > FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  > AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  > LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  > OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  > THE SOFTWARE.

- **`Triangulation` (`include/CNA/Extended/Triangulation/`, ported from MonoGame.Extended's
  `Math/Triangulation/*.cs`: `Vertex`, `LineSegment`, `Triangle`, `CyclicalList`,
  `IndexableCyclicalLinkedList`, `Triangulator`)** originates from nickgravelyn's
  [Triangulator](https://github.com/nickgravelyn/Triangulator) (an ear-clipping
  triangulation implementation), per every one of those six upstream files' own header
  comments: `MIT Licensed: https://github.com/nickgravelyn/Triangulator`.
  **Provenance note (2026-07-13):** unlike the SlimMath case above, MonoGame.Extended's
  own file headers here only assert "MIT Licensed" and a URL — they don't reproduce the
  original license text or a copyright holder name. At the time of this port,
  `https://github.com/nickgravelyn/Triangulator` returned HTTP 404 (via both the web UI
  and the GitHub API), and no mirror or archived copy of its `LICENSE` file could be
  located, so the exact original copyright line could not be independently verified
  verbatim. Multiple independent sources (all six of MonoGame.Extended's own upstream file
  headers, consistently, plus third-party indexes of the project found via web search) agree
  the project was MIT licensed. Given that consistent attestation, the standard MIT License
  template below is used, crediting nickgravelyn as the original author — if the original
  repository resurfaces, this entry should be updated with the verbatim original text:
  > Copyright (c) nickgravelyn
  >
  > Permission is hereby granted, free of charge, to any person obtaining a copy
  > of this software and associated documentation files (the "Software"), to deal
  > in the Software without restriction, including without limitation the rights
  > to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  > copies of the Software, and to permit persons to whom the Software is
  > furnished to do so, subject to the following conditions:
  >
  > The above copyright notice and this permission notice shall be included in
  > all copies or substantial portions of the Software.
  >
  > THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  > IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  > FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  > AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  > LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  > OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  > THE SOFTWARE.

If a future ported file's upstream `.cs` header credits another project by name the same
way `Angle.cs` does, add it here too rather than folding it into the "courtesy
attribution" list below, which is for inspiration-only credits with no code actually
carried into this repository.

## Code directly derived from other permissively-licensed (non-MIT) projects

Unlike the MIT-licensed cases above, `Bag.cs`'s own upstream header credits a project
under a **2-clause-BSD-style license, not MIT** — a genuinely different license family,
kept in its own section for clarity rather than folded into the "MIT-licensed" heading
above.

- **`Bag<T>` (`include/CNA/Extended/Collections/Bag.hpp`, ported from MonoGame.Extended's
  `Collections/Bag.cs`)** is GAMADU.COM's C# port of thelinuxlich's
  [artemis_CSharp](https://github.com/thelinuxlich/artemis_CSharp) `Bag<T>`
  (`Artemis_XNA_INDEPENDENT/Utils/Bag.cs`), an Entity-Component-System framework's
  utility collection. **Provenance note (2026-07-13):** verified directly against the
  original `artemis_CSharp` repository (still live and accessible, unlike the
  `nickgravelyn/Triangulator` case above) — fetched
  `Artemis_XNA_INDEPENDENT/Utils/Bag.cs` from that repository and confirmed its
  copyright header matches MonoGame.Extended's embedded copy verbatim, word for word.
  Full license text, Copyright (c) 2013 GAMADU.COM:
  > Redistribution and use in source and binary forms, with or without modification, are
  > permitted provided that the following conditions are met:
  >
  > 1. Redistributions of source code must retain the above copyright notice, this list
  >    of conditions and the following disclaimer.
  >
  > 2. Redistributions in binary form must reproduce the above copyright notice, this
  >    list of conditions and the following disclaimer in the documentation and/or other
  >    materials provided with the distribution.
  >
  > THIS SOFTWARE IS PROVIDED BY GAMADU.COM 'AS IS' AND ANY EXPRESS OR IMPLIED
  > WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
  > AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL GAMADU.COM OR
  > CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  > CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
  > OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
  > AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  > (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  > SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  >
  > The views and conclusions contained in the software and documentation are those of
  > the authors and should not be interpreted as representing official policies, either
  > expressed or implied, of GAMADU.COM.

  This 2-clause-BSD-style license is permissive and compatible with inclusion in an
  overall MIT-licensed project (it requires the notice above to be preserved in
  redistributions, which this entry does) — it does not require this project or
  `Bag.hpp` specifically to itself be BSD-licensed; `cna-extended`'s own `LICENSE`
  (MIT) applies to this project's original code and to MonoGame.Extended's own
  modifications layered on top, same as everywhere else in this project.

## Upstream lineage (courtesy attribution)

MonoGame.Extended's own README credits the following as inspiration or direct origin for
parts of the code this project ports. Other than `Angle` (see above), none of these
projects' code is used directly here — only MonoGame.Extended's own (MIT-licensed)
implementation is ported — but the lineage is worth preserving:

- The `Particles` module is derived from Matthew-Davey's
  [Mercury Particle Engine](https://github.com/Matthew-Davey/mercury-particle-engine).
- [2D XNA Primitives](https://bitbucket.org/C3/2d-xna-primitives/wiki/Home) by John
  McDonald influenced parts of the `Math`/`Shapes` primitives.
- [LibGDX](https://libgdx.badlogicgames.com) was a general inspiration across the
  library.

## This project

cna-extended itself is licensed under the MIT License; see `LICENSE`.
