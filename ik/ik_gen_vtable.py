import sys
import re
import copy
import string
from os import walk, makedirs
from os.path import join, isdir, basename, realpath, dirname

in_file_name = sys.argv[1]
out_file_name = sys.argv[2]
implementations = list()
interfaces = list()
file_content = list()

out_directory = dirname(realpath(out_file_name))
if not isdir(out_directory):
    makedirs(out_directory)


class IKMethod(object):
    def __init__(self, impl_name, ret_type, name, arg_types):
        self.impl_name = impl_name
        self.name = name
        self.ret_type = ret_type
        self.arg_types = arg_types
        self.is_final = False
        self.is_constructor = False

    def get_func_name(self):
        return "ik_{}_{}".format(self.impl_name, self.name)

    def get_func_signature(self):
        return "{} {}({})".format(self.ret_type, self.get_func_name(), ", ".join(self.arg_types))


class IKImplementation(object):
    def __init__(self, derived_name, base_name=None):
        self.derived_name = derived_name
        self.base_name = base_name
        self.methods = list()
        self.befores = list()
        self.afters = list()
        self.harnesses = list()

    def get_API_defs(self):
        return "#define IK_{}_IMPL \\\n    {}".format(
            self.derived_name.upper(),
            ", \\\n    ".join(x.get_func_name() for x in self.harnesses)
        )

    def get_IMPL_defs(self):
        return "\n".join(("IK_PRIVATE_API " if "static " not in x else "") + x + ";" for harness in self.harnesses for x in harness.get_func_signatures()
                         if harness.impl_name == self.derived_name)

    def get_harness_bodies(self):
        return "\n".join(x.get_harness_body() for x in self.harnesses if x.impl_name == self.derived_name)


class IKHarness(object):
    def __init__(self, name):
        self.impl_name = name
        self.methods = list()

    def get_func_name(self):
        return (self.methods[0].get_func_name() if len(self.methods) == 1
                else "ik_{}_harness_{}".format(self.impl_name, self.methods[0].name))

    def get_harness_signature(self):
        if len(self.methods) == 1:
            return ""
        return "{} {}({})".format(self.methods[0].ret_type, self.get_func_name(), ", ".join(self.methods[0].arg_types))

    def get_harness_retval_func_name(self):
        return self.get_func_name() + "_return_value"

    def get_harness_retval_signature(self):
        arglist = [self.methods[0].ret_type] * len(self.methods)
        return self.methods[0].ret_type + " " + self.get_harness_retval_func_name() + "(" + ", ".join(arglist) + ")"

    def get_func_signatures(self):
        sigs = list()
        for method in self.methods:
            if method.impl_name == self.impl_name:
                sigs.append(method.get_func_signature())
                if method.ret_type != "void" and len(self.methods) > 1 and not self.is_constructor:
                    sigs.append("static inline " + self.get_harness_retval_signature())
        return sigs

    def get_harness_body(self):
        if len(self.methods) == 1:
            return ""
        arglist = self.methods[0].arg_types
        rettype = self.methods[0].ret_type
        paramlist = [arg.split()[-1] if arg != "void" else "" for arg in arglist]
        func_calls = [x.get_func_name() + "(" + ", ".join(paramlist) + ")" for x in self.methods]
        body = "static inline {} {}({})".format(rettype, self.get_func_name(), ", ".join(arglist)) + "\n{\n"
        if self.is_constructor:
            if rettype != "ik_ret":
                print("Constructor must return ik_ret, not " + rettype)
                sys.exit(-1)
            body += "    ik_ret result;\n"
            goto_tags = [x.impl_name + "_failed" for x in self.methods]
            for constructor_call, goto_tag in zip(func_calls, goto_tags):
                body += "    if ((result = " + constructor_call + ") != IK_OK) goto " + goto_tag + ";\n"
            body += "    return IK_OK;\n"
            for constructor_call, goto_tag in zip(func_calls[-2::-1] + [None], goto_tags[::-1]):
                if constructor_call is None:
                    body += "    " + goto_tag + " : return result;\n"
                else:
                    body += "    " + goto_tag + " : " + constructor_call.replace("construct", "destruct") + ";\n"
            body += "}"
        else:
            if rettype != "void":
                tmpvars = string.ascii_lowercase[:len(func_calls)]
                for func_call, tmpvar in zip(func_calls, tmpvars):
                    body += "    " + rettype + " " + tmpvar + " = " + func_call + ";\n"
                body += "    return " + self.get_harness_retval_func_name() + "("
                body += ", ".join(tmpvars)
                body += ");"
            else:
                body += "    " + ";\n    ".join(func_calls) + ";"
            body += "\n}"
        return body


def find_impl_or_interface(name):
    for i in implementations:
        if i.derived_name == name:
            return i
    for i in interfaces:
        if i.derived_name == name:
            return i


def iterate_source_files(directory, extensions):
    for root, directories, files in walk(directory, followlinks=True):
        for file in files:
            if any(file.endswith(x) for x in extensions):
                yield join(root, file)
        for directory in directories:
            iterate_source_files(join(root, directory), extensions)


def parse_interface(f, interface):
    scopes_opened = 0
    comments_opened = 0
    accumulate = str()
    for line in f:
        # handle scopes
        if "{" in line:
            scopes_opened += 1
            continue
        if "}" in line:
            scopes_opened -= 1
            continue
        if scopes_opened <= 0:
            return line

        # handle comments
        if comments_opened and "*/" in line:
            comments_opened -= 1
            continue
        if "/*" in line:
            comments_opened += 1
        if comments_opened != 0 or "//" in line:
            continue

        # handle multi-line function signatures
        accumulate += line.strip()
        if not ";" in accumulate:
            continue

        try:
            ret, name, args = re.match(r"(.*)\(\*(.*)\)\((.*)\);", accumulate).groups()
            args = [x.strip() for x in args.split(",") if x.strip()]
        except:
            print("Failed to parse function signature \"{}\"".format(accumulate))
            sys.exit(-1)
        interface.methods.append(IKMethod(interface.derived_name, ret.strip(), name.strip(), [x.strip() for x in args]))
        accumulate = str()


def parse_implementation(f, impl):
    scopes_opened = 0
    for line in f:
        # handle scopes
        if "{" in line:
            scopes_opened += 1
            continue
        if "}" in line:
            scopes_opened -= 1
            continue
        if scopes_opened <= 0:
            return line

        # only keywords we care about
        if not any(x in line for x in ("IK_OVERRIDE", "IK_BEFORE", "IK_AFTER", "IK_FINAL", "IK_FINAL_BEFORE", "IK_FINAL_AFTER", "IK_CONSTRUCTOR", "IK_DESTRUTOR")):
            continue

        try:
            overrides = re.match(r"IK_OVERRIDE\((.*)\)", line.strip()).group(1)
            for override in (x.strip() for x in overrides.split(",") if x.strip()):
                impl.methods.append(IKMethod(impl.derived_name, None, override, None))  # None's are patched later
            continue
        except:
            pass

        try:
            befores = re.match(r"(IK_BEFORE|IK_DESTRUCTOR)\((.*)\)", line.strip()).group(1)
            for before in (x.strip() for x in befores.split(",") if x.strip()):
                impl.befores.append(IKMethod(impl.derived_name, None, before, None))  # None's are patched later
            continue
        except:
            pass

        try:
            afters = re.match(r"IK_AFTER\((.*)\)", line.strip()).group(1)
            for after in (x.strip() for x in afters.split(",") if x.strip()):
                impl.afters.append(IKMethod(impl.derived_name, None, after, None))  # None's are patched later
            continue
        except:
            pass

        try:
            finals = re.match(r"IK_FINAL\((.*)\)", line.strip())
            finals = finals.group(1)
            for final in (x.strip() for x in finals.split(",") if x.strip()):
                override = IKMethod(impl.derived_name, None, final, None)  # None's are patched later
                override.is_final = True
                impl.methods.append(override)
            continue
        except:
            pass

        try:
            befores = re.match(r"IK_FINAL_BEFORE\((.*)\)", line.strip()).group(1)
            for before in (x.strip() for x in befores.split(",") if x.strip()):
                new_before = IKMethod(impl.derived_name, None, before, None)  # None's are patched later
                new_before.is_final = True
                impl.befores.append(new_before)
            continue
        except:
            pass

        try:
            afters = re.match(r"IK_FINAL_AFTER\((.*)\)", line.strip()).group(1)
            for after in (x.strip() for x in afters.split(",") if x.strip()):
                new_after = IKMethod(impl.derived_name, None, after, None)  # None's are patched later
                new_after.is_final = True
                impl.afters.append(new_after)
            continue
        except:
            pass

        try:
            afters = re.match(r"IK_CONSTRUCTOR\((.*)\)", line.strip()).group(1)
            for after in (x.strip() for x in afters.split(",") if x.strip()):
                new_after = IKMethod(impl.derived_name, None, after, None)  # None's are patched later
                new_after.is_constructor = True
                impl.afters.append(new_after)
            continue
        except:
            pass

        print("Failed to parse statement \"{}\"".format(line.strip()))
        sys.exit(-1)


def collect_from_file(file_name):
    global file_content
    write_to_file_content = False
    if basename(file_name) == basename(in_file_name):
        write_to_file_content = True
    with open(file_name, "r") as f:
        for line in f:
            while line is not None and "IK_INTERFACE" in line:
                try:
                    interface_args = re.match(r"IK_INTERFACE\((.*)\)", line).group(1)
                except:
                    print("Couldn't parse IK_INTERFACE in file " + file_name)
                    sys.exit(-1)
                interface_args = [x.strip() for x in interface_args.split(",") if x.strip()]
                if len(interface_args) != 1:
                    print("Wrong number of arguments to IK_INTERFACE in file " + file_name)
                    sys.exit(-1)
                interface = IKImplementation(interface_args[0], None)
                line = parse_interface(f, interface)
                interfaces.append(interface)

            while line is not None and "IK_IMPLEMENT" in line:
                try:
                    impl_args = re.match(r"IK_IMPLEMENT\((.*)\)", line).group(1)
                except:
                    print("Couldn't parse IK_IMPLEMENT in file " + file_name)
                    sys.exit(-1)
                impl_args = [x.strip() for x in impl_args.split(",") if x.strip()]
                if len(impl_args) == 2:
                    if (impl_args[0] == impl_args[1]):
                         print("Can't implement when the derived type is the same as the base in file " + file_name)
                         sys.exit(-1)
                else:
                    print("Wrong number of arguments to IK_IMPLEMENT in file " + file_name)
                    sys.exit(-1)
                impl = IKImplementation(*impl_args)
                line = parse_implementation(f, impl)
                implementations.append(impl)
                if write_to_file_content:
                    file_content.append("__implementation__" + impl.derived_name)

            if not write_to_file_content or line is None:
                continue
            file_content.append(line.strip("\n"))

def patch_impl_method_signatures():
    for impl in implementations:
        chain = [impl]
        base = find_impl_or_interface(impl.base_name)
        if base is None and impl.base_name:
            print("Error: Unknown base \"{}\" for implementation \"{}\"".format(impl.base_name, impl.derived_name))
            sys.exit(-1)
        while base is not None:
            chain.append(base)
            next_base = find_impl_or_interface(base.base_name)
            if next_base is None and base.base_name:
                print("Error: Unknown base \"{}\" for implementation \"{}\"".format(base.base_name, base.derived_name))
                sys.exit(-1)
            base = next_base

        if len(chain) < 2:
            if impl.base_name is None:
                print("Warning: Interface {} is never implemented!".format(impl.derived_name))
            continue

        # index 0 should be interface, 1 should be base, 2 onwards should be derivatives
        chain = chain[::-1]

        # interfaces and bases cannot have befores or afters
        if len(chain[0].befores) > 0 or len(chain[0].afters) > 0:
            print("Error: " + chain[0].derived_name + ": Interfaces cannot have any befores or afters")
            sys.exit(-1)
        if len(chain[1].befores) > 0 or len(chain[1].afters) > 0:
            print("Error: Base implementations cannot have any befores or afters")
            sys.exit(-1)

        # During parsing the return types and arg types were set to None. Patch
        # those in from the interface (which does have that data)
        for chain_part in chain[1:]:
            for i in chain[0].methods:
                for d in chain_part.methods:
                    if i.name == d.name:
                        d.ret_type = i.ret_type
                        d.arg_types = i.arg_types
            for i in chain[0].methods:
                for d in chain_part.befores:
                    if i.name == d.name:
                        d.ret_type = i.ret_type
                        d.arg_types = i.arg_types
            for i in chain[0].methods:
                for d in chain_part.afters:
                    if i.name == d.name:
                        d.ret_type = i.ret_type
                        d.arg_types = i.arg_types

        # generate the harness table by filling it with all of the base methods
        # note that the base implementation won't have any methods specified,
        # because it expects to override everything. That's why we copy the
        # methods from the interface instead but overwrite the implementer name
        # with the base's name
        harness_table = list()
        for iface_method in chain[0].methods:
            for base_method in chain[1].methods:
                if base_method.name == iface_method.name:
                    copied_method = copy.copy(base_method)
                    break
            else:
                copied_method = copy.copy(iface_method)
            copied_method.impl_name = chain[1].derived_name  # base is implementing interface
            harness = IKHarness(copied_method.impl_name)
            harness.is_constructor = copied_method.is_constructor
            harness.methods.append(copied_method)
            harness_table.append(harness)

        for chain_idx in list(range(2, len(chain))):  # for every derived implementation...
            for harness in harness_table:

                # find index of method in base method list
                for derived_method in chain[chain_idx].methods:
                    if derived_method.name == harness.methods[0].name:
                        # Check if this method is being overridden by a new implementer
                        # if so, overwrite it
                        if derived_method.impl_name not in [x.impl_name for x in harness.methods]:
                            # can't override if it's marked as final
                            if any(x.is_final for x in harness.methods):
                                print("Error: " + derived_method.impl_name + " is trying to override final method " + derived_method.name)
                                sys.exit(-1)
                            harness.methods = [copy.copy(derived_method)]  # back to just one entry in the harness, since it's being overridden
                            harness.impl_name = derived_method.impl_name
                            harness.is_constructor = derived_method.is_constructor

                for derived_method in chain[chain_idx].afters:
                    if derived_method.name == harness.methods[0].name:
                        if derived_method.impl_name not in [x.impl_name for x in harness.methods]:
                            # can't override if it's marked as final
                            if any(x.is_final for x in harness.methods):
                                print("Error: " + derived_method.impl_name + " is trying to override final method " + derived_method.name)
                                sys.exit(-1)
                            harness.methods.append(copy.copy(derived_method))
                            harness.impl_name = derived_method.impl_name
                            harness.is_constructor = derived_method.is_constructor

                for derived_method in chain[chain_idx].befores:
                    if derived_method.name == harness.methods[0].name:
                        if derived_method.impl_name not in [x.impl_name for x in harness.methods]:
                            # can't override if it's marked as final
                            if any(x.is_final for x in harness.methods):
                                print("Error: " + derived_method.impl_name + " is trying to override final method " + derived_method.name)
                                sys.exit(-1)
                            harness.methods.insert(0, copy.copy(derived_method))
                            harness.impl_name = derived_method.impl_name
                            harness.is_constructor = derived_method.is_constructor

        impl.harnesses = harness_table

def generate_header(file_name, lines, out_file):
    with open(out_file, "w") as f:
        # Writer header found in every IK header file
        guard_str = "IK_VTABLE_" + basename(out_file).replace(".h", "").upper() + "_H"
        f.write(
            "/* WARNING: This file was auto-generated by " + sys.argv[0] + " */\n"
            "#ifndef " + guard_str + "\n"
            "#define " + guard_str + "\n"
            "\n"
            "#include \"ik/config.h\"\n"
            "\n"
            "C_HEADER_BEGIN\n"
            "\n"
        )

        # Write any existing lines that were in the file to begin with
        # and fill in our generated defines
        for line in lines:
            if "__implementation__" not in line:
                f.write(line + "\n")
                continue

            impl_name = line.replace("__implementation__", "")
            impl = find_impl_or_interface(impl_name)
            f.write(impl.get_IMPL_defs() + "\n")
            f.write(impl.get_API_defs() + "\n\n")
            f.write(impl.get_harness_bodies() + "\n\n")

        f.write(
            "C_HEADER_END\n"
            "\n"
            "#endif /* " + guard_str + " */\n"
        )


for file in iterate_source_files(".", [".c", ".h", ".v"]):
    collect_from_file(file)

patch_impl_method_signatures()
generate_header(in_file_name, file_content, out_file_name)
