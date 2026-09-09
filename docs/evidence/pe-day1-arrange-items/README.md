# DAY1-10 Arrange Items live verification

Native run27, 2026-09-06. Binary SHA256:
`07781970f1d297d9a1d8922a5d8dc4e55f6f3e66fb1432d11285059e4592e1ef`.
Normal keyboard input only; game memory and framebuffers inspected read-only.

After the first Eve battle, opened Arrange Items at frame18510, cancelled it,
then opened/cancelled/confirmed each of its three submenu options. Sorts at
18680,19010,19380 preserve the full inventory multiset and equipped item IDs
256/257 while changing their positions. `verification.json` records the exact
orders and frames. Screenshots show the menu, all three submenus, and the
final sorted inventory. Medicine1 subsequently heals16→45HP at frame20840.

Original-execution equivalence is established separately by
`pc_port/tools/pe_inventory_sort_oracle.py` and `test_inventory_sort.h`;
these screenshots show the native execution, not a side-by-side retail capture.
Full Day1, other menu/save paths, and Windows runtime acceptance remain open.
