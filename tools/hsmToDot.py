# Author: Frederic Hamel (fhamel@gmail.com)

import os
import sys
import re
import pprint
import binascii

def PrintUsage():
	print """
Parses cpp file(s) containing an HSM and outputs dot format text that can be used to render it.
	
Usage: {} <filespec>
	""".format(os.path.basename(sys.argv[0]))

DOT_LEFT_RIGHT = False
DOT_USE_COLOR = True
DOT_FONT = "Helvetica"
	
STRIP_COMMENTS_RE = re.compile("(.*?)//(.*)", re.IGNORECASE)
NEW_STATE_RE = re.compile("struct\s+(\w+)\s*:\s*(?:public)?\s*(\w+)", re.IGNORECASE)
INNER_TRANSITION_RE = re.compile("InnerTransition\s*<\s*([^\s]*?)\s*>\s*\(", re.IGNORECASE)
INNER_ENTRY_TRANSITION_RE = re.compile("InnerEntryTransition\s*<\s*([^\s]*?)\s*>\s*\(", re.IGNORECASE)
SIBLING_TRANSITION_RE = re.compile("SiblingTransition\s*<\s*([^\s]*?)\s*>\s*\(", re.IGNORECASE)
TYPEDEFS_RE = re.compile("typedef.*\s+(?P<statename>\w+)\s+(?P<alias>\w+)\s*;", re.IGNORECASE)

TAG_REUSABLE_STATE = "@PLOTHSM_REUSABLE"
IGNORED_STATE_NAMES = ["Args"]

INNER_TRANSITION = 1
INNER_ENTRY_TRANSITION = 2
SIBLING_TRANSITION = 3

REPLACE_UNDERSCORES_WITH_DASHES = False

def Info(message):
	sys.stderr.write(message + "\n")

def Warn(message):
	sys.stderr.write(message + "\n")

def Indent(level):
	#@LAME: there HAS to be a better way of doing this in python, I just can't come up with it right now
	result = ""
	for i in range(level):
		result = result + "  "
	return result

uid = 1
def GetUid():
	global uid
	uid = uid + 1
	return uid

class Transition:
	def __init__(self, transitionType, targetStateOrTargetStateName):
		self.Type = transitionType
		#Info(str(type(targetStateOrTargetStateName)))
		if type(targetStateOrTargetStateName) == str:
			#@TODO: either unset this when the TargetState is set, or do some python magic with __getattr__ to fetch the real value from the state
			self.TargetStateName = targetStateOrTargetStateName
			self.TargetState = None
		elif isinstance(targetStateOrTargetStateName, State):
			self.TargetState = targetStateOrTargetStateName
			self.TargetStateName = targetStateOrTargetStateName.Name
		else:
			raise Exception("Unknown type in transition creation")
			
		self.IsLegal = True
	def __str__(self):
		return {INNER_TRANSITION: "I", INNER_ENTRY_TRANSITION: "E", SIBLING_TRANSITION: "S"}[self.Type] + ":" + self.TargetStateName
		
class State:
	def __init__(self, name, baseName):
		self.Name = name
		self.BaseName = baseName
		self.Base = None
		self._transitions = []
		self.Rank = -1
		self.IsBase = False
		self.IsRoot = False
		self.IsReusable = False
		self.IsProxy = False
		self.Cluster = "" #@TODO: remove this
		self.Clusters = []
		self.Parents = []
		self.Children = []
		self.IsHidden = False
		self.IncomingSiblings = [] # states who have a sibling transition to this one
		#Info("New state: %s" % self)
	
	def Inners(self, mustBeLegal = True):
		return filter(lambda x: (x.Type == INNER_TRANSITION) and (x.IsLegal or not mustBeLegal), self._transitions)

	def InnerEntries(self, mustBeLegal = True):
		return filter(lambda x: (x.Type == INNER_ENTRY_TRANSITION) and (x.IsLegal or not mustBeLegal), self._transitions)

	def Siblings(self, mustBeLegal = True):
		return filter(lambda x: (x.Type == SIBLING_TRANSITION) and (x.IsLegal or not mustBeLegal), self._transitions)
			   
	def Transitions(self):
		return self._transitions
		
	def ReferencedStates(self):
		return [x.TargetState for x in self._transitions]

	def AddTransition(self, transitionType, targetStateOrTargetStateName):
		t = Transition(transitionType, targetStateOrTargetStateName)
		#Info(str(t))
		self._transitions.append(t)
		
	def IsReferenced(self):
		return len(self.Parents) > 0 or len(self.IncomingSiblings) > 0
		
	def IsVisible(self):
		result = len(self._transitions) > 0 or self.IsReferenced()
		result = result and not self.IsHidden
		return result

	def __str__(self):
		value = "State %s" % self.Name
		
		printTransitions = False
		printRank = False
		printParents = True
		printChildren = True
		printClusters = True

		if printRank:
			value += " Rank: %d" % (self.Rank)
		if printParents:
			for s in self.Parents:
				value += " Parent: " + s.Name
		if printChildren:
			for s in self.Children:
				value += " Child: " + s.Name
		if printTransitions:
			for t in self._transitions:
				value += " " + str(t)
		if printClusters:
			for c in self.Clusters:
				value += " " + c
		return value
		
class Hsm:
	def __init__(self):
		self._states = {}
		self._aliases = {}
	
	def AddState(self, stateName, stateBaseName):
		self._states[stateName] = State(stateName, stateBaseName)
		return self._states[stateName]
		
	def States(self):
		return self._states.values()

	def _ValidateStateReferences(self):
		for state in self._states.values():
			for transition in state.Transitions():
				if not self.GetStateByNameOrAlias(transition.TargetStateName):
					raise Exception("State %s references unknown state %s" % (state.Name, transition.TargetStateName))
			# We allow deriving from unknown states		
			#if not self._states.has_key(state.BaseName):
				#raise Exception("State %s derives from unknown state %s" % (state.Name, state.BaseName))
	   
	def _AssignStates(self):
		for state in self._states.values():
			for transition in state.Transitions():
				transition.TargetState = self.GetStateByNameOrAlias(transition.TargetStateName)
			if self.GetStateByNameOrAlias(state.BaseName):
				state.Base = self.GetStateByNameOrAlias(state.BaseName)
				state.Base.IsBase = True
  
	def _ValidateCycleTopologyAndAssignRanksForState(self, state, rank, statesOnStack = None, visitedStates = None):
		# Really complex function which tries to make sure there are no illegal cycles in the state machine.
		# VisitedStates[] is partially redundant (since the information can be inferred from the rank value in states),
		# but lives on for the moment for validation purposes.
		if statesOnStack is None:
			statesOnStack = []
		if visitedStates is None:
			visitedStates = []

		#Info("  _ValidateCycleTopologyAndAssignRanksForState: %s" % state.Name)
		#for visitedState in visitedStates:
			#Info("	" + str(visitedState))
			
		state.Rank = rank
		visitedStates.append(state)
		for t in state.Siblings():
			if not t.TargetState in statesOnStack:
				if not t.TargetState in visitedStates: 
					if not t.TargetState.Rank == -1:
						raise Exception("Rank should be -1 if state was not visited (%s)" % (str(t.TargetState)))
					self._ValidateCycleTopologyAndAssignRanksForState(t.TargetState, rank, statesOnStack, visitedStates)
				else:
					if t.TargetState.Rank == -1:
						raise Exception("Rank should never be -1 if state was visited (%s)" % (str(t.TargetState)))
					if t.TargetState.Rank < rank:
						self._ValidateCycleTopologyAndAssignRanksForState(t.TargetState, rank, statesOnStack, visitedStates)
			else:
				Warn("Invalid topology detected: %s siblings to %s, which is a parent" % (state.Name, t.TargetStateName))
				t.IsLegal = False;
		statesOnStack.append(state)
		for t in state.Inners() + state.InnerEntries():
			if not t.TargetState in statesOnStack:
				self._ValidateCycleTopologyAndAssignRanksForState(t.TargetState, rank + 1, statesOnStack, visitedStates)
			else:
				Warn("Invalid topology detected: %s performs an inner/innerentry to %s, which is a parent" % (state.Name, t.TargetStateName))
				t.IsLegal = False;
		statesOnStack.remove(state)
		
	def _ValidateCycleTopologyAndAssignRanksForRoots(self):
		for root in self.GetRoots():
			#Info("Visiting root: " + root.Name)
			self._ValidateCycleTopologyAndAssignRanksForState(root, 0)

	def _AssignParentsAndChildrenForState(self, state, statesOnStack = None):
		if statesOnStack is None:
			statesOnStack = []
			
		if len(statesOnStack) > 0:
			goOn = False
			curParent = statesOnStack[-1]
			if not state in curParent.Children:
				if curParent in state.Parents:
					raise Exception("Child is not in parent's children, but parent is in child's parents")
				if curParent != state:
					state.Parents.append(curParent)
					curParent.Children.append(state)
					goOn = True
				else:
					Warn("Topology warning: state %s is its own parent" % (state.Name))
			else:
				if not curParent in state.Parents:
					raise Exception("Parent is not in child's parents, but child is in parent's children")
		else:
			goOn = True

		if goOn:
			for t in state.Siblings():
				self._AssignParentsAndChildrenForState(t.TargetState, statesOnStack)
			statesOnStack.append(state)
			for t in state.Inners() + state.InnerEntries():
				self._AssignParentsAndChildrenForState(t.TargetState, statesOnStack)
			statesOnStack.remove(state)
		   
	def _AssignParentsAndChildrenForRoots(self):
		for root in self.GetRoots():
			self._AssignParentsAndChildrenForState(root)

	def _AssignIncomingSiblings(self):
		for state in self._states.values():
			for transition in state.Siblings():
				if not state in transition.TargetState.IncomingSiblings:
					transition.TargetState.IncomingSiblings.append(state)

	# Recall that this function does not rely on parent/child information being set for proper traversal order
	def _VisitStateOnceDepthFirst(self, state, predicate, statesOnStack = None, visitedStates = None):
		if statesOnStack is None:
			statesOnStack = []
		if visitedStates is None:
			visitedStates = []

		predicate(state, statesOnStack)
		visitedStates.append(state)
		statesOnStack.append(state)
		for t in state.Inners() + state.InnerEntries():
			if not t.TargetState in visitedStates:
				self._VisitStateOnceDepthFirst(t.TargetState, predicate, statesOnStack, visitedStates)
		statesOnStack.remove(state)
		for t in state.Siblings():
			if not t.TargetState in visitedStates:
				self._VisitStateOnceDepthFirst(t.TargetState, predicate, statesOnStack, visitedStates)

	def VisitAllStatesOnceDepthFirst(self, predicate):
		for root in self.GetRoots():
			self._VisitStateOnceDepthFirst(root, predicate)

	def _VisitStateOnceBaseFirst(self, state, predicate, visitedStates):
		if not state in visitedStates:
			#Info("Visiting %s" % str(state))
			visitedStates.append(state)
			#Info(pprint.pformat(visitedStates))

			if state.Base:
				#Info("Visiting base %s" % str(state.Base))
				self._VisitStateOnceBaseFirst(state.Base, predicate, visitedStates)

			predicate(state)
			for nextState in state.ReferencedStates():
				self._VisitStateOnceBaseFirst(nextState, predicate, visitedStates)

	def VisitAllStatesOnceBaseFirst(self, predicate):
		visitedStates = []
		for state in self._states.values():
			#Info("VisitAllStatesOnceBaseFirst %s" % str(state))
			self._VisitStateOnceBaseFirst(state, predicate, visitedStates)

	@staticmethod
	def _InheritBaseTransitionsForState(state):
		if state.Base:
			for t in state.Base.Transitions():
				#Info("Inheriting base transition %s" % t)
				state.AddTransition(t.Type, t.TargetState)

	def _InheritBaseTransitions(self):
		self.VisitAllStatesOnceBaseFirst(self._InheritBaseTransitionsForState)
			
	@staticmethod
	def _AssignClusterForState(state, statesOnStack):
		if len(statesOnStack) > 0:
			curParent = statesOnStack[-1]
			if state.Name.startswith(curParent.Name) \
				or (curParent.Cluster != "" and state.Name.startswith(curParent.Cluster)) \
				or state.IsProxy:
				if curParent.Cluster == "":
					curParent.Cluster = curParent.Name
				#Info("state %s assigned to parent's cluster %s" % (state.Name, curParent.Cluster))
				state.Cluster = curParent.Cluster
			clustersFromParent = [x for x in curParent.Clusters if state.Name.startswith(x)]
			state.Clusters = clustersFromParent
			# Possibly add a new cluster if our name starts with our parents'
			if state.Name.startswith(curParent.Name) and not curParent.Name in state.Clusters:
				state.Clusters.append(curParent.Name)
				curParent.Clusters.append(curParent.Name)

	def _AssignClusters(self):
		self.VisitAllStatesOnceDepthFirst(self._AssignClusterForState)

	def _MarkRoots(self):
		roots = self._states.values()
		for state in self._states.values():
			for transition in state.Transitions():
				if transition.TargetState in roots and transition.TargetState != state:
					roots.remove(transition.TargetState)
		for potentialRoot in roots:
			if not potentialRoot.IsBase:
				potentialRoot.IsRoot = True
	
	def GetRoots(self):
		return filter(lambda x: x.IsRoot, self._states.values())

	def GetAllReachableStatesFromState(self, state):
		states = []
		def GatherStates(state, statesOnStack):
			if not state in states:
				states.append(state)
		self._VisitStateOnceDepthFirst(state, GatherStates)
		return states

	def _ResetStateRanks(self):
		for state in self._states.values():
			state.Rank = -1

	def _ValidateRootTopology(self):
		rootSets = {}
		for root in self.GetRoots():
			states = self.GetAllReachableStatesFromState(root)
			#Info("States reachable by root %s:" % root.Name)
			#for state in states:
				#Info("  " + state.Name)
			rootSets[root] = set(states)
		
		ok = True
		for root in rootSets.keys():
			for root2 in rootSets.keys():
				if root != root2:
					# if not rootSets[root].isdisjoint(rootSets[root2]): # requires 2.6
					if len(rootSets[root].intersection(rootSets[root2])) > 0:
						Warn("Topology error: roots %s and %s can reach common states" % (root.Name, root2.Name))
						ok = False
		if not ok:
			raise Exception("Invalid topology detected.")

	def _CreateSurrogateRoots(self):
		for root in self.GetRoots():
			# Create a unique root to which nobody will transition.  Ensures all subsequence algorithms will work properly.
			surrogateRoot = self.AddState("SurrogateRoot" + str(GetUid()), "")
			surrogateRoot.IsRoot = True
			root.IsRoot = False
			surrogateRoot.AddTransition(INNER_ENTRY_TRANSITION, root)
			surrogateRoot.IsHidden = True
	
	def _ReplaceReusableStatesWithProxies(self):
		for state in self._states.values():
			for transition in state.Transitions():
				if transition.TargetState.IsReusable:
					proxyState = self.AddState(transition.TargetState.Name + str(GetUid()), "")
					transition.TargetState = proxyState
					transition.TargetStateName = proxyState.Name
					proxyState.IsProxy = True

	def Finalize(self):
		self._ValidateStateReferences()
		self._AssignStates()
		self._InheritBaseTransitions() # Only requires transition targets and state bases to be resolved
		self._MarkRoots()
		self._ValidateRootTopology() # finds duplicate DAG roots; does NOT requires parent information
		self._CreateSurrogateRoots() # creates "fake" roots if required, to ensure that we really have a tree; requires valid root topology, does not require parent/child info
		self._ValidateCycleTopologyAndAssignRanksForRoots() # marks illegal cycles, required for marking parents/children; requires no duplicate DAG roots
		self._ReplaceReusableStatesWithProxies() # requires no cycles
		self._ResetStateRanks()
		self._ValidateCycleTopologyAndAssignRanksForRoots() # re-rank states, because reusable states have now been split out
		self._AssignParentsAndChildrenForRoots() # marks parents/children, required no cycles
		self._AssignIncomingSiblings() # marks incoming siblings; required (among other things) to determine which states are referenced
		self._AssignClusters()
				   
	def GetStateByName(self, name):
		if self._states.has_key(name):
			return self._states[name]
		else:
			return None

	def GetStateByNameOrAlias(self, name):
		if self._states.has_key(name):
			return self._states[name]
		elif self._aliases.has_key(name) and self._states.has_key(self._aliases[name]):
			return self._states[self._aliases[name]]
		else:
			return None
			
	def GetStatesByRank(self, rank):
		return filter(lambda x: x.Rank == rank, self._states.values())

	def GetMaxRank(self):
		return reduce(lambda x, y: max(x, y), (x.Rank for x in self._states.values()))
			
	def GetStatesByCluster(self, cluster):
		return filter(lambda x: x.Cluster == cluster, self._states.values())

	def GetClusters(self):
		clusters = []
		def GatherClusters(state, statesOnStack):
			if state.Cluster != "" and not state.Cluster in clusters:
				clusters.append(state.Cluster)
		self.VisitAllStatesOnceDepthFirst(GatherClusters)
		return clusters

	def __str__(self):
		s = ""
		for state in self._states.values():
			s += str(state) + "\n"
		return s
			
def ParseHsm(filespec, hsm):
	curState = None
	file = open(filespec)
	for fullLine in file.readlines():
		lineNoComments = fullLine
		m = STRIP_COMMENTS_RE.match(fullLine)
		if m != None:
			lineNoComments = m.group(1)
			
		m = TYPEDEFS_RE.search(fullLine) 
		if m:
			# last two identifiers in typedef statement -> potential state alias
			hsm._aliases[m.group('alias')] = m.group('statename')

		m = NEW_STATE_RE.search(lineNoComments)
		if m != None:
			stateName = m.group(1)
			if not stateName in IGNORED_STATE_NAMES:
				stateBaseName = m.group(2)
				if hsm.GetStateByName(stateName) != None:
					raise Exception("Duplicate state: %s" % (stateName))
				curState = hsm.AddState(stateName, stateBaseName)
				if fullLine.find(TAG_REUSABLE_STATE) != -1:
					curState.IsReusable = True

		if curState != None:
			m = INNER_TRANSITION_RE.search(lineNoComments)
			if m != None:
				curState.AddTransition(INNER_TRANSITION, m.group(1))
			m = INNER_ENTRY_TRANSITION_RE.search(lineNoComments)
			if m != None:
				curState.AddTransition(INNER_ENTRY_TRANSITION, m.group(1))
			m = SIBLING_TRANSITION_RE.search(lineNoComments)
			if m != None:
				curState.AddTransition(SIBLING_TRANSITION, m.group(1))
		
	file.close()
		   
	return hsm

def GetLabelForState(state):
	return "%s (%d)" % (state.Name, state.Rank)
	#return state.Name
	#return "%s (%d)" % (state.Name, state.IsReusable)
	#return ("%s (:%s) (C:%s) (%d)" % (state.Name, state.BaseName, state.Cluster, state.Rank))
	#return ("%s (C:%s)" % (state.Name, " ".join(state.Clusters)))
	
def GetAttributesForState(hsm, state):
	label = GetLabelForState(state)
	if REPLACE_UNDERSCORES_WITH_DASHES:
		label = label.replace("_", "-")
	result = "label=\"%s\"" % label
	if state.IsProxy:
		result = result + ",shape=parallelogram"

	if DOT_USE_COLOR:
		# We select a unique hue per cluster
		clusterHash = binascii.crc32("".join(state.Clusters)) & 0xffffffff
		H = float((clusterHash + sys.maxint/4) % sys.maxint) / sys.maxint

		S = 0.5

		# For the value (brightness), scale by rank so nodes of higher rank are brighter.
		#@TODO: Compute as ratio of state rank to cluster min/max rank so that we get the full
		#       range of brightness within a cluster.
		rankRatio = float(state.Rank) / hsm.GetMaxRank()
		minV = 0.3
		maxV = 0.85
		V = minV + rankRatio * (maxV - minV)

		result += ',style=filled, color="{} {} {}"'.format(H, S, V)
		result += ',fontcolor=white'

	result += ',fontname=%s' % (DOT_FONT)
	return result

def GetAttributesForTransition(transition):
	weight = 1
	color = "black"
	style = "solid"
	if transition.Type == INNER_TRANSITION or transition.Type == INNER_ENTRY_TRANSITION:
		if len(transition.TargetState.Parents) == 1:
			weight = 100
		else:
			weight = 1
		if transition.Type == INNER_ENTRY_TRANSITION:
			style = "bold"
	if transition.Type == SIBLING_TRANSITION:
		weight = 50
		style = "dotted"
	if not transition.IsLegal:
		color = "red"
	
	return ("[style=\"%s\",weight=%d,color=\"%s\"]" % (style, weight, color))
	#return ("[style=\"%s\",weight=%d,color=\"%s\",label=\"%d\"]" % (style, weight, color, weight))

def GetAttributesForChildPositioningEdge(transition):
	weight = 1
	if len(transition.TargetState.Parents) == 1:
		weight = 300
	attributes = "[weight=%d,penwidth=0.0,arrowhead=none]" % weight
	#attributes = "[weight=%d,color=\"gray\",label=\"%d\"]" % (weight, weight)
	return attributes

#prohibited = ["Alive", "Grounded"]
prohibited = []
allowed = [] #["Gatling"]

def ShouldPrintState(a):
	if not a.IsVisible():
		return False
		
	global prohibited
	global allowed
	for i in prohibited:
		if i.lower() in a.Name.lower():
			return False

	# This reads funny, but it means "if the allowed list is empty, then we presume all states to be allowed"
	if not allowed:
		return True

	for i in allowed:
		if i.lower() in a.Name.lower():
			return True

	return False

def ShouldPrintTransition(a, b):
	return ShouldPrintState(a) and ShouldPrintState(b)

class ClusterMapElement:
	def __init__(self):
		self.SubClusters = []
		self.States = []
	
	def __str__(self):
		return "sc: %s s: %s" % (" ".join(self.SubClusters), " ".join(self.States))

#@LAME: this data structure is not awesome
def BuildClusterMap(hsm):
	clusterMap = {"": ClusterMapElement()}
	for state in hsm.States():
		for i in range(len(state.Clusters)):
			if i > 0:
				parentCluster = state.Clusters[i - 1]
			else:
				parentCluster = ""
			
			childCluster = state.Clusters[i]
			
			if not clusterMap.has_key(childCluster):
				clusterMap[childCluster] = ClusterMapElement()
			
			if not childCluster in clusterMap[parentCluster].SubClusters:
				clusterMap[parentCluster].SubClusters.append(childCluster)
		
		# Add the state to the proper cluster's state array
		if len(state.Clusters) > 0:
			stateCluster = state.Clusters[-1]
		else:
			stateCluster = ""
			
		clusterMap[stateCluster].States.append(state)
	return clusterMap

def PrintDotFile(hsm):
	print("digraph G {")
	if DOT_LEFT_RIGHT:
		print("  rankdir=LR;")
	print("  nodesep=0.4;")
	print("  fontname=%s;" % (DOT_FONT))

	clusterMap = BuildClusterMap(hsm)

	def InfoCluster(hsm, clusterMap, clusterName):
		Info("Cluster: %s" % clusterName)
		for subCluster in clusterMap[clusterName].SubClusters:
			Info("Subcluster of %s: %s" % (clusterName, subCluster))
			InfoCluster(hsm, clusterMap, subCluster)
	
	def PrintCluster(hsm, clusterMap, clusterName, depth = 0):
		indent = Indent(depth)
		element = clusterMap[clusterName]
		
		if clusterName != "":
			print("%ssubgraph cluster_%s" % (indent, clusterName))
			print("%s{" % indent)
			print("%s  label=\"%s\";" % (indent, clusterName))
			print("%s  labeljust=left;" % (indent))

		for subCluster in element.SubClusters:
			PrintCluster(hsm, clusterMap, subCluster, depth + 1)

		clusterStates = element.States
		for rank in range(hsm.GetMaxRank() + 1):
			rankStatesInCluster = [x for x in clusterStates if x.Rank == rank]
			if len(rankStatesInCluster) > 0:
				print("%s  {\n%s    rank = same;" % (indent, indent))
				for state in rankStatesInCluster:
					if state.IsVisible() and ShouldPrintState(state):
						print("%s    %s [%s];" % (indent, state.Name, GetAttributesForState(hsm, state)))
				print("%s  }" % indent)
				
		if clusterName != "":
			print("%s}" % indent)
		
	PrintCluster(hsm, clusterMap, "")

	print
	
	for state in hsm.States():
		for transition in state.Transitions():
			if ShouldPrintTransition(state, transition.TargetState):
				attributes = GetAttributesForTransition(transition)
				print("  %s -> %s %s;" % (state.Name, hsm.GetStateByNameOrAlias(transition.TargetStateName).Name, attributes))
		for child in state.Children:
			if ShouldPrintTransition(state, child):
				attributes = GetAttributesForChildPositioningEdge(transition)
				print("  %s -> %s %s;" % (state.Name, hsm.GetStateByNameOrAlias(child.Name).Name, attributes))

	print ("}")

# Deploy on each console
def main(argv = None):
	if argv is None:
		argv = sys.argv
	
	if len(argv) < 2:
		PrintUsage()
		return 0

	filespec = argv[1]
	
	hsm = Hsm()
	for file in argv[1:]:
		ParseHsm(file, hsm)
	hsm.Finalize()

	PrintDotFile(hsm)

if __name__ == "__main__":
	sys.exit(main())
