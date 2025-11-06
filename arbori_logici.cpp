#include <iostream>
#include <vector>
#include <string>
#include <set>

using namespace std;

vector<char> op = { '&', '|', '>' };

struct expression {
	string s;
	bool expanded = false;
	bool negated = false;

	static bool is_simple(string val) {
		if (val.length() == 1 && isupper(val[0]))
			return true;
		return false;
	}

	static bool is_negated(string val) {
		if (val.length() == 2)
			if (val[0] == '!' && isupper(val[1]))
				return true;
			else return false;
		if (val[0] == '!' && val[1] == '(' && val[val.size() - 1] == ')') {
			int x = 1;
			for (int i = 2; i < val.size(); i++) {
				if (val[i] == '(')
					x++;
				else if (val[i] == ')')
					x--;
				if (x == 0) {
					if (i == val.size() - 1) {
						return true;
					}
					break;
				}
			}
			return false;
		}
		return false;
	}

	static void strip_parenthesis(string& val) {
		if (!(val[0] == '(' && val[val.size() - 1] == ')'))
			return;
		int x = 1;
		for (int i = 1; i < val.size(); i++) {
			if (val[i] == '(')
				x++;
			else if (val[i] == ')')
				x--;
			if (x == 0) {
				if (i == val.size() - 1) {
					val = val.substr(1, val.size() - 2);
					strip_negation(val);
					strip_parenthesis(val);
					return;
				}
				break;
			}
		}
	}

	static void strip_negation(string& s) {
		if (!is_negated(s)) return;
		s = s.substr(1, s.size() - 1);
		strip_parenthesis(s);
		strip_negation(s);
	}

	expression(string val) {
		// delete double negations, because they make everything very difficult
		while (int p = val.find("!!")) {
			if (p == string::npos) break;
			val.replace(p, 2, "");
		}
		if (is_negated(val)) {
			negated = true;
			strip_negation(val);
		}
		else strip_parenthesis(val); // strip_negation already calls strip_parenthesis
		if (is_simple(val))
			expanded = true;
		s = val;
	}
};

class Tree {
private:
	vector<vector<expression>> val_group;
	vector<int> par;

	set<int> active_groups;
public:
	int get_parent(int group) const {
		return par[group];
	}

	int add_group(int parent) {
		val_group.push_back({});
		par.push_back(parent);
		return val_group.size() - 1;
	}
	void _set_active(int group) {
		active_groups.insert(group);
	}
	bool is_active(int group) {
		return active_groups.count(group) > 0;
	}

	void add(expression e, int group, bool negate = false) {
		if (negate) e.negated = !e.negated;
		val_group[group].push_back(e);
	}

	int nrgroups() const {
		return val_group.size();
	}

	vector<expression>& group(int i) {
		return val_group[i];
	}

	bool check_contradictions(int group) {
		for (expression e : val_group[group]) {
			if (!expression::is_simple(e.s)) continue;
			for (expression e2 : val_group[group]) {
				if (!expression::is_simple(e2.s)) continue;
				if (e.negated != e2.negated && e.s == e2.s) {
					//cout << "Contradiction between: " << e.s << ", and " << e2.s << '\n';
					return true;
				}
			}
		}
		for (expression e : val_group[group]) {
			if (!expression::is_simple(e.s)) continue;
			int parent = par[group];
			while (parent != -1) {
				for (expression e2 : val_group[parent]) {
					if (!expression::is_simple(e2.s)) continue;
					if (e.negated != e2.negated && e.s == e2.s) {
						//cout << "Contradiction between: " << e.s << ", and " << e2.s << '\n';
						return true;
					}
				}
				parent = par[parent];
			}
		}
		return false;
	}

	bool is_child(int child, int ancestor) {
		while (child != -1) {
			if (child == ancestor)
				return true;
			child = par[child];
		}
		return false;
	}

	void branch1(int root, expression e) {
		set<int> useful_branches;
		for (int group : active_groups)
			if (is_child(group, root))
				useful_branches.insert(group);
		set<int> new_actives;
		for (int group : useful_branches) {
			int new_group = add_group(group);
			add(e, new_group);
			new_actives.insert(new_group);
		}
		for (int group : useful_branches)
			active_groups.erase(group);
		for (int group : new_actives)
			if (!check_contradictions(group))
				active_groups.insert(group);
	}

	void branch1(int root, expression e1, expression e2) {
		set<int> useful_branches;
		for (int group : active_groups)
			if (is_child(group, root))
				useful_branches.insert(group);
		set<int> new_actives;
		for (int group : useful_branches) {
			int new_group = add_group(group);
			add(e1, new_group);
			add(e2, new_group);
			new_actives.insert(new_group);
		}
		for (int group : useful_branches)
			active_groups.erase(group);
		for (int group : new_actives)
			if (!check_contradictions(group))
				active_groups.insert(group);
	}

	void branch2(int root, expression e1, expression e2) {
		set<int> useful_branches;
		for (int group : active_groups)
			if (is_child(group, root))
				useful_branches.insert(group);
		set<int> new_actives;
		for (int group : active_groups) {
			int new_group1 = add_group(group);
			int new_group2 = add_group(group);
			add(e1, new_group1);
			add(e2, new_group2);
			new_actives.insert(new_group1);
			new_actives.insert(new_group2);
		}
		for (int group : useful_branches)
			active_groups.erase(group);
		for (int group : new_actives)
			if (!check_contradictions(group))
				active_groups.insert(group);
	}

	bool done() const {
		return active_groups.empty();
	}
};

int precedence(char op) {
	switch (op) {
	case '(':
	case ')':
		return 3;
	case '&':
	case '|':
	case '>':
		return 1;
	default:
		throw("invalid op");
	}
}

int main()
{
	freopen("x.in", "r", stdin);
	Tree tree;
	int group = tree.add_group(-1);
	tree._set_active(group);
	while (true) {
		string s;
		getline(cin, s);

		if (s != "=>") {
			tree.add(s, group);
		}
		else {
			getline(cin, s);
			tree.add(s, group, true);
			break;
		}
	}
	if (tree.check_contradictions(group)) {
		cout << "VALID!!\n";
		return 0;
	}
	//cout << " ********************** \n";
	bool ok = true;
	while (ok && !tree.done()) {
		ok = false;
		for (int curr_group = 0; curr_group < tree.nrgroups(); curr_group++) {
			for (expression& exp : tree.group(curr_group)) {
				if (exp.expanded) continue;
				exp.expanded = true;
				//cout << exp.s << ": \n";
				ok = true;
				int index = 0;
				string s = "(" + exp.s + ")";
				int l = s.length();
				int i = 0;
				vector<char> operands;
				vector<pair<char, int>> operators; // operator and position
				int pos_of_last_applied = -1;
				while (i < l) {
					if (isupper(s[i])) {
						operands.push_back(s[i]);
					}
					else if (s[i] != '!') {
						if (s[i] == ')') {
							while (operators[operators.size() - 1].first != '(') {
								/* x = */operands.pop_back();
								/* y = */operands.pop_back();
								pos_of_last_applied = operators.back().second;
								/* op=*/operators.pop_back();
								operands.push_back('X');
							}
							operators.pop_back();
						}
						else if (s[i] == '(') {
							operators.push_back({ s[i], i });
						}
						else {
							int prec = precedence(s[i]);
							while (prec <= precedence(operators[operators.size() - 1].first)) {
								if (operators[operators.size() - 1].first == '(') {
									break;
								}
								/*y =*/operands.pop_back();
								/*x =*/operands.pop_back();
								pos_of_last_applied = operators.back().second;
								/*op=*/ operators.pop_back();
								operands.push_back('X');
							}
							operators.push_back({ s[i], i });
						}
					}
					i += 1;
				}
				if (pos_of_last_applied == -1) {
					cout << "SPECIAL CASE: " << s << '\n';
					string ans = "";
					for (char c : s) {
						if (c != '(' && c != ')')
							ans.push_back(c);
					}
					//cout << "ans= " << ans << '\n';
					tree.branch1(curr_group, ans);
				}
				else {
					//cout << pos_of_last_applied << ' ' << s[pos_of_last_applied] << '\n
					//cout << "is negated: " << exp.negated << '\n';
					string s1;
					for (int i = 1; i < pos_of_last_applied; i++)
						s1.push_back(s[i]);
					expression e1 = s1;
					if (exp.negated) e1.negated = !e1.negated;
					string s2;
					for (int i = pos_of_last_applied + 1; i < s.size() - 1; i++)
						s2.push_back(s[i]);
					expression e2 = s2;
					if (exp.negated) e2.negated = !e2.negated;
					//cout << s1 << "     " << s2 << '\n';
					char op = s[pos_of_last_applied];
					if (op == '&') {
						if (!exp.negated) {
							tree.branch1(curr_group,
								e1, e2);
						}
						else {
							tree.branch2(curr_group,
								e1,
								e2);
						}
					}
					else if (op == '|') {
						if (!exp.negated) {
							tree.branch2(curr_group,
								e1,
								e2);
						}
						else {
							tree.branch1(curr_group,
								e1, e2);
						}
					}
					else if (op == '>') {
						if (!exp.negated) {
							tree.branch2(curr_group,
								e1,
								e2);
						}
						else {
							tree.branch1(curr_group,
								e1, e2);
						}
					}
				}
				//cout << "----------------------\n";
			}
		}
	}
	if (tree.done()) {
		cout << "Valid\n";
	}
	else {
		cout << "Nevalid\n";
	}
	/*
	for (int i = 0; i < tree.nrgroups(); i++) {
		cout << ";;;;;;;;;;;;;;;;;;;;;\n";
		cout << "Group: " << i << " : ";
		if (tree.is_active(i)) {
			cout << "Active\n";
		}
		else {
			cout << "Not active\n";
		}
		cout << "Children: ";
		for (int j = 0; j < tree.nrgroups(); j++) {
			if (tree.get_parent(j) == i)
				cout << j << ", ";
		}
		cout << '\n';
		cout << "Expressions: \n";
		for (expression e : tree.group(i)) {
			cout << e.s << " : ";
			if (e.expanded)
				cout << "Expanded\n";
			else cout << "Not expanded\n";
		}
	}
	*/
	return 0;
}

