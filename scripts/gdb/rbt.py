import gdb

class TColors:
    BLUE = "\033[94m"
    RED = "\033[91m"
    GREEN = "\033[92m"
    GRAY = "\033[90m"
    CBLINK = "\033[5m"
    END = "\033[0m"


def apply_color(text: str, color: TColors) -> str:
    return color + text + TColors.END


NULLPTR = gdb.parse_and_eval("(struct rbtree_node *)0")


class RBSubTree:
    def __init__(self, ptr: gdb.Value):
        self.ptr = ptr

    @property
    def left(self):
        if self.ptr == NULLPTR:
            return None

        if self.ptr["left"] != 0:
            return RBSubTree(self.ptr["left"])
        else:
            return RBSubTree(NULLPTR)

    @property
    def right(self):
        if self.ptr == NULLPTR:
            return None

        if self.ptr["right"] != 0:
            return RBSubTree(self.ptr["right"])
        else:
            return RBSubTree(NULLPTR)


    @property
    def data(self):
        if self.ptr == NULLPTR:
            return None
        return self.ptr["data"]

    def has_parent(self) -> bool:
        if self.ptr == NULLPTR:
            return None

        return self.ptr["parent"] != 0

    def has_colour_field(self) -> bool:
        fields = self.ptr.dereference().type.fields()
        names = map(lambda f: f.name, fields)
        return "colour" in names

    def is_leaf(self) -> bool:
        return self.ptr == 0

    def is_red(self) -> bool:
        # If a node is NULL-ptr, then it's black.
        if self.ptr == 0:
            return False

        if self.has_colour_field():
            color = self.ptr["colour"]
            return str(color) == "RBTREE_RED"
        else:
            parent = self.ptr["parent"]
            return (int(parent) & 1) == 1

    def __str__(self):
        red_str = apply_color("Red", TColors.RED)
        black_str = apply_color("Black", TColors.GRAY)

        state_arr = []
        if not self.has_parent() and self.ptr != NULLPTR:
            state_arr.append("Root")

        state_arr.append("Addr:" + str(self.ptr))
        state_arr.append("Color:" + (red_str if self.is_red() else black_str))

        if self.ptr != NULLPTR:
            state_arr.append("Data: " + str(self.data))

        state = "; ".join(state_arr)
        if self.ptr == 0:
            return apply_color(state, TColors.GRAY)
        return state


class PrintRBSubtreeCmd(gdb.Command):
    """Prints Red-Black subtree. First argument must be of type 'struct rbtree_node *'"""

    def __init__(self):
        super(PrintRBSubtreeCmd, self).__init__("rbsubt_print", gdb.COMMAND_USER)

    def complete(self, text, word):
        return gdb.COMPLETE_SYMBOL

    def dump_tree(self, subtree: RBSubTree):
        for i in range(self.PRINT_LEVEL):
            print("|\t", end="")
        print(subtree)

        self.PRINT_LEVEL += 1
        if subtree.left is not None:
            self.dump_tree(subtree.left)
        if subtree.right is not None:
            self.dump_tree(subtree.right)
        self.PRINT_LEVEL -= 1

    def invoke(self, arg, from_tty):
        args = gdb.string_to_argv(arg)

        val = gdb.parse_and_eval(args[0])
        if str(val.type) != "struct rbtree_node *":
            print("Expected pointer argument of (struct rbtree_node *)")
            return

        self.PRINT_LEVEL = 0
        self.dump_tree(RBSubTree(val))


PrintRBSubtreeCmd()
