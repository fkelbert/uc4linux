Successfully created mutex [pefLogger]
[    threadUtils][DEBUG] Successfully created mutex [pdpMutex]
pdpConstructor successfully finished
[  testOperators][WARN ] Starting testpdp application for operator-tests
[    socketWin32][DEBUG] Winsock initialized!
[    socketWin32][INFO ] Successfully created win-TCP-socket for port=[9983]
[    socketUtils][INFO ] Opened TCP-socket (fd=[1848], port=[9983])
[    threadUtils][DEBUG] Created new thread with handler=[00417da6] => threadID=[0]
[    socketUtils][DEBUG] pefSocket successfully allocated for port=[9983] with handler=[00407422]
[            pdp][DEBUG] PDP interfaces initialized.
[            pdp][INFO ] PDP initialized
[  testOperators][TRACE] PDP started with result=[SUCCESS]
[  testOperators][TRACE] PEP registration => [SUCCESS]
[            pdp][DEBUG] Found a registered PEP for this ID
[actionDescStore][TRACE] Action description [action1] not found, inserting description
[         action][TRACE] Created action description [action1]
[actionDescStore][TRACE] Action description [action1] not found, returning NULL
[actionDescStore][TRACE] Adding action description [action1] to store
[            pdp][DEBUG] ActionDescription [action1] successfully found/added to store
[            pdp][DEBUG] Action [action1] successfully associated to PEP [testpdp]
[  testOperators][TRACE] Action registration => [SUCCESS]
[            pdp][DEBUG] Found a registered PEP for this ID
[actionDescStore][TRACE] Action description [action2] not found, inserting description
[         action][TRACE] Created action description [action2]
[actionDescStore][TRACE] Action description [action2] not found, returning NULL
[actionDescStore][TRACE] Adding action description [action2] to store
[            pdp][DEBUG] ActionDescription [action2] successfully found/added to store
[            pdp][DEBUG] Action [action2] successfully associated to PEP [testpdp]
[  testOperators][TRACE] Action registration => [SUCCESS]
[  testOperators][TRACE] PXP registration => [SUCCESS]
[            pdp][DEBUG] Found a registered PXP for this ID
[actionDescStore][TRACE] Action description [notify] not found, inserting description
[         action][TRACE] Created action description [notify]
[actionDescStore][TRACE] Action description [notify] not found, returning NULL
[actionDescStore][TRACE] Adding action description [notify] to store
[            pdp][DEBUG] ActionDescription [notify] successfully found/added to store
[            pdp][DEBUG] Execution action [notify] successfully associated to PXP [testpdp]
[  testOperators][TRACE] Executor registration => [SUCCESS]
[            pdp][TRACE] pdpDeployPolicy - c:\local\rd\usr\home\raindrop\workspace\pef\src\main\xml\examples\testDuring.xml
[    pdpInternal][DEBUG] Loading mechanisms from file: c:\local\rd\usr\home\raindrop\workspace\pef\src\main\xml\examples\testDuring.xml (for mechanism=[NULL])
[       xmlUtils][INFO ] XML file [c:\local\rd\usr\home\raindrop\workspace\pef\src\main\xml\examples\testDuring.xml] is valid according to pef-schema
[actionDescStore][TRACE] Action description [event.homeapp.controller.backend] not found, inserting description
[         action][TRACE] Created action description [event.homeapp.controller.backend]
[actionDescStore][TRACE] Action description [event.homeapp.controller.backend] not found, returning NULL
[actionDescStore][TRACE] Adding action description [event.homeapp.controller.backend] to store
[    pdpInternal][DEBUG] ActionDescription [event.homeapp.controller.backend] successfully added to store
[    pdpInternal][TRACE] NO PEP interface specified for action [event.homeapp.controller.backend]...
[actionDescStore][TRACE] === Action description store:
[         action][TRACE] === Action description: [notify]
[         action][TRACE]                  Class: [usage]
[         action][TRACE] ================================
[         action][TRACE] === Action description: [event.homeapp.controller.backend]
[         action][TRACE]                  Class: [usage]
[         action][TRACE] ================================
[         action][TRACE] === Action description: [action1]
[         action][TRACE]                  Class: [usage]
[         action][TRACE] ================================
[         action][TRACE] === Action description: [action2]
[         action][TRACE]                  Class: [usage]
[         action][TRACE] ================================
[actionDescStore][TRACE] ================================
[    pdpInternal][DEBUG] Parsing actionDescriptions... [SUCCESS]
[      mechanism][TRACE] Mechanism Type: preventiveMechanism
[      mechanism][TRACE] MechanismFullName=[c:\local\rd\usr\home\raindrop\workspace\pef\src\main\xml\examples\testDuring.xml#testDuring]
[    threadUtils][DEBUG] Successfully created mutex [testDuring]
[      mechanism][TRACE] Successfully allocated mechanism "testDuring" in namespace "c:\local\rd\usr\home\raindrop\workspace\pef\src\main\xml\examples\testDuring.xml" (@ 1338041126500000)
[      mechanism][TRACE] xmlParseTimestepSize - timestepSize: 3000000 us
[actionDescStore][TRACE] Action description [action1] found in store
[  pefEventMatch][INFO ] Creating event matching operator action=[action1] match_index=[ongoing] match_try=[true]
[         action][TRACE] Param description [val1] not found, returning NULL
[         action][TRACE] Successfully added parameter description [val1] of type [string] to action [action1]
[      paramDesc][TRACE] Param value [value1] not found, returning NULL
[      paramDesc][TRACE] Param value [value1] not found, returning NULL
[      paramDesc][TRACE] Adding value [value1] to parameter description [val1] of action [action1]
[  pefEventMatch][INFO ] no parameter type given in policy; defaulting to PARAM_SRING!
[  pefEventMatch][DEBUG] Adding param [val1] to eventMatch
[  pefEventMatch][TRACE] Adding parameter matching [val1][value1][string][false] to event
[  pefEventMatch][INFO ] Successfully parsed XML and created eventMatch operator referencing action "action1"
[  testOperators][INFO ] pepSubscribeNative invoked for name=[action1] and unsubscribe=[0]...
[      mechanism][DEBUG] PEP subscription for event name=[action1]=>[0]
[actionDescStore][TRACE] Action description [action1] found in store
[  pefEventMatch][INFO ] Creating event matching operator action=[action1] match_index=[ongoing] match_try=[true]
[         action][TRACE] Param description [val1] found
[      paramDesc][TRACE] Param value [value1] found
[  pefEventMatch][INFO ] no parameter type given in policy; defaulting to PARAM_SRING!
[  pefEventMatch][DEBUG] Adding param [val1] to eventMatch
[  pefEventMatch][TRACE] Adding parameter matching [val1][value1][string][false] to event
[  pefEventMatch][INFO ] Successfully parsed XML and created eventMatch operator referencing action "action1"
[  testOperators][INFO ] pepSubscribeNative invoked for name=[action1] and unsubscribe=[0]...
[      condition][DEBUG] PEP subscription for event name=[action1]=>[0]
[      condition][TRACE] Subscribing this eventMatch condition operand to the list of condition nodes
[   oslOperators][TRACE] Operator type: EVENTMATCH
[   oslOperators][TRACE]           op1: action1
[   oslOperators][TRACE] Operator type: DURING
[   oslOperators][TRACE]           op1: 7
[   oslOperators][TRACE]           op2: SECONDS
[   oslOperators][TRACE]  ==> interval: 2 timesteps 
[   oslOperators][TRACE]           op3: EVENTMATCH
[      condition][DEBUG] Condition formula with 2 nodes
[      condition][TRACE]  DURING [amount=7, timeunit=SECONDS => interval=2 timesteps]
[      condition][TRACE]   EVENTMATCH [actionName=action1]
[authorizationAc][DEBUG] Successfully allocated new mechanism_action (response: 0, delay: 0)
[      mechanism][TRACE] Successfully parsed mechanism testDuring (size: 131 bytes)
[      mechanism][TRACE] Mechanism: 
[      mechanism][TRACE]          name: testDuring
[      mechanism][TRACE]  timestepSize: 3000000
[      mechanism][TRACE]         start: 1338041126500000
[      mechanism][TRACE]   last update: 1338041126500000
[      mechanism][TRACE]  cur timestep: 0
[    pdpInternal][INFO ] Successfully added mechanism "testDuring" to hashtable
[    pdpInternal][DEBUG] Starting update thread for mechanisms=[testDuring] with usleep=[3000000]
[    threadUtils][DEBUG] Created new thread with handler=[0040c250] => threadID=[1868770928]
[    threadUtils][DEBUG] Thread successfully started
[      mechanism][INFO ] Started mechanism update thread usleep=300000 us
[    pdpInternal][INFO ] Successfully loaded mechanisms from file: c:\local\rd\usr\home\raindrop\workspace\pef\src\main\xml\examples\testDuring.xml
[            pdp][TRACE] pdpDeployPolicy - loading policy returned [SUCCESS]
[  testOperators][TRACE] deploy returned=[0]
[      mechanism][WARN ] ////////////////////////////////////////////////////////////////////////////////////////////////////////////
[      mechanism][DEBUG] mechanismUpdate - [testDuring] Null-Event updating 0. timestep at interval of 3000000 us
[      condition][TRACE] conditionUpdate - updating with event=[00000000]
[        oslEval][TRACE]  [testDuring] - evaluating EVENT    node => 0
[        oslEval][TRACE]  [testDuring] - evaluating DURING   history available=[false]
[        oslEval][TRACE] [DURING] Setting position [0] to [0]
[        oslEval][TRACE] [DURING] circArray=[0 0 0 ]
[        oslEval][TRACE]  [testDuring] - evaluating DURING   node => 0
[      condition][DEBUG]  [testDuring] - condition value=[false]
[      mechanism][INFO ] mechanismUpdate - [testDuring] event=[0] timestep=[0] value=[false]
[      mechanism][TRACE] mechanismUpdate - [testDuring] -----------------------------------------------------------
[      mechanism][WARN ] ////////////////////////////////////////////////////////////////////////////////////////////////////////////
[      mechanism][DEBUG] mechanismUpdate - [testDuring] Null-Event updating 1. timestep at interval of 3000000 us
[      condition][TRACE] conditionUpdate - updating with event=[00000000]
[        oslEval][TRACE]  [testDuring] - evaluating EVENT    node => 0
[        oslEval][TRACE]  [testDuring] - evaluating DURING   history available=[false]
[        oslEval][TRACE] [DURING] Setting position [1] to [0]
[        oslEval][TRACE] [DURING] circArray=[0 0 0 ]
[        oslEval][TRACE]  [testDuring] - evaluating DURING   node => 0
[      condition][DEBUG]  [testDuring] - condition value=[false]
[      mechanism][INFO ] mechanismUpdate - [testDuring] event=[0] timestep=[1] value=[false]
[      mechanism][TRACE] mechanismUpdate - [testDuring] -----------------------------------------------------------
[      mechanism][WARN ] ////////////////////////////////////////////////////////////////////////////////////////////////////////////
[      mechanism][DEBUG] mechanismUpdate - [testDuring] Null-Event updating 2. timestep at interval of 3000000 us
[      condition][TRACE] conditionUpdate - updating with event=[00000000]
[        oslEval][TRACE]  [testDuring] - evaluating EVENT    node => 0
[        oslEval][TRACE]  [testDuring] - evaluating DURING   history available=[true]
[        oslEval][TRACE] [DURING] circArray=[0 0 0 ]
[        oslEval][TRACE] [DURING] Setting position [2] to [0]
[        oslEval][TRACE] [DURING] circArray=[0 0 0 ]
[        oslEval][TRACE]  [testDuring] - evaluating DURING   node => 0
[      condition][DEBUG]  [testDuring] - condition value=[false]
[      mechanism][INFO ] mechanismUpdate - [testDuring] event=[0] timestep=[2] value=[false]
[      mechanism][TRACE] mechanismUpdate - [testDuring] -----------------------------------------------------------
[      mechanism][WARN ] ////////////////////////////////////////////////////////////////////////////////////////////////////////////
[      mechanism][DEBUG] mechanismUpdate - [testDuring] Null-Event updating 3. timestep at interval of 3000000 us
[      condition][TRACE] conditionUpdate - updating with event=[00000000]
[        oslEval][TRACE]  [testDuring] - evaluating EVENT    node => 0
[        oslEval][TRACE]  [testDuring] - evaluating DURING   history available=[true]
[        oslEval][TRACE] [DURING] circArray=[0 0 0 ]
[        oslEval][TRACE] [DURING] Setting position [0] to [0]
[        oslEval][TRACE] [DURING] circArray=[0 0 0 ]
[        oslEval][TRACE]  [testDuring] - evaluating DURING   node => 0
[      condition][DEBUG]  [testDuring] - condition value=[false]
[      mechanism][INFO ] mechanismUpdate - [testDuring] event=[0] timestep=[3] value=[false]
[      mechanism][TRACE] mechanismUpdate - [testDuring] -----------------------------------------------------------
[      mechanism][WARN ] ////////////////////////////////////////////////////////////////////////////////////////////////////////////
[      mechanism][DEBUG] mechanismUpdate - [testDuring] Null-Event updating 4. timestep at interval of 3000000 us
[      condition][TRACE] conditionUpdate - updating with event=[00000000]
[        oslEval][TRACE]  [testDuring] - evaluating EVENT    node => 0
[        oslEval][TRACE]  [testDuring] - evaluating DURING   history available=[true]
[        oslEval][TRACE] [DURING] circArray=[0 0 0 ]
[        oslEval][TRACE] [DURING] Setting position [1] to [0]
[        oslEval][TRACE] [DURING] circArray=[0 0 0 ]
[        oslEval][TRACE]  [testDuring] - evaluating DURING   node => 0
[      condition][DEBUG]  [testDuring] - condition value=[false]
[      mechanism][INFO ] mechanismUpdate - [testDuring] event=[0] timestep=[4] value=[false]
[      mechanism][TRACE] mechanismUpdate - [testDuring] -----------------------------------------------------------
[            pdp][TRACE] receiving event (0): [<event isTry="true" action="action1" index="ALL"><parameter name="val1" value="value1"/></event>]
[actionDescStore][TRACE] Action description [action1] found in store
[       pefEvent][TRACE] Creating event action=[action1] index=[ongoing] is_try=[true]
[       pefEvent][TRACE] Event id=[0] [action1][ongoing][true] created
[       pefEvent][DEBUG] no parameter type given for event; defaulting to PARAM_SRING!
[         action][TRACE] Param description [val1] found
[       pefParam][TRACE] Adding parameter [val1][value1][type:0] to event
[       pefEvent][DEBUG] Successfully parsed XML and created event referencing action "action1"
[            pdp][DEBUG] Searching for subscribed condition nodes for event=[action1]; subscribed nodes=[1]
[            pdp][DEBUG] conditionTriggerEvent - checking condition eventMatch=[action1] for levent=[action1]
[  pefEventMatch][DEBUG] Matching event [0]
[  pefEventMatch][TRACE] Event index matches operator [ongoing]~=[ongoing]
[  pefEventMatch][TRACE] Event is_try matches operator [true]=[true]
[  pefEventMatch][TRACE] Event action matches [action1]=[action1]
[  pefParamMatch][DEBUG] matchEventParameters - comparing trigger event (nParam=[1]) with desired event(nParam=[1])
[  pefParamMatch][DEBUG] param [val1]: param value for matching, calling specific matching function...
[  pefParamMatch][DEBUG] Matching parameter using common compare
[       pefParam][DEBUG] Event parameter [val1] found
[  pefParamMatch][DEBUG] Parameter [val1] match
[  pefEventMatch][DEBUG] Event matches operator
[            pdp][TRACE]  evaluated EVENT    node => 1
[            pdp][DEBUG] Searching for triggered mechanism for event=[action1]; subscribed mechanisms=[1]
[            pdp][DEBUG] mechanismTriggerEvent - checking mech=[testDuring] for levent=[action1]
[  pefEventMatch][DEBUG] Matching event [0]
[  pefEventMatch][TRACE] Event index matches operator [ongoing]~=[ongoing]
[  pefEventMatch][TRACE] Event is_try matches operator [true]=[true]
[  pefEventMatch][TRACE] Event action matches [action1]=[action1]
[  pefParamMatch][DEBUG] matchEventParameters - comparing trigger event (nParam=[1]) with desired event(nParam=[1])
[  pefParamMatch][DEBUG] param [val1]: param value for matching, calling specific matching function...
[  pefParamMatch][DEBUG] Matching parameter using common compare
[       pefParam][DEBUG] Event parameter [val1] found
[  pefParamMatch][DEBUG] Parameter [val1] match
[  pefEventMatch][DEBUG] Event matches operator
[            pdp][INFO ] mechanismTriggerEvent - Found matching mechanism -> mechanism_name=[testDuring]
[authorizationAc][DEBUG] Successfully allocated new mechanism_action (response: 1, delay: 0)
[      condition][TRACE] conditionUpdate - updating with event=[00e60740]
[  pefEventMatch][DEBUG] Matching event [0]
[  pefEventMatch][TRACE] Event index matches operator [ongoing]~=[ongoing]
[  pefEventMatch][TRACE] Event is_try matches operator [true]=[true]
[  pefEventMatch][TRACE] Event action matches [action1]=[action1]
[  pefParamMatch][DEBUG] matchEventParameters - comparing trigger event (nParam=[1]) with desired event(nParam=[1])
[  pefParamMatch][DEBUG] param [val1]: param value for matching, calling specific matching function...
[  pefParamMatch][DEBUG] Matching parameter using common compare
[       pefParam][DEBUG] Event parameter [val1] found
[  pefParamMatch][DEBUG] Parameter [val1] match
[  pefEventMatch][DEBUG] Event matches operator
[        oslEval][TRACE]  [testDuring] - evaluating EVENT    node => 1
[        oslEval][TRACE]  [testDuring] - evaluating DURING   history available=[true]
[        oslEval][TRACE] [DURING] circArray=[0 0 0 ]
[        oslEval][TRACE]  [testDuring] - evaluating DURING   node => 0
[      condition][DEBUG]  [testDuring] - condition value=[false]
[            pdp][TRACE] condition NOT satisfied, doing nothing...
[    pdpInternal][DEBUG] notifyEvent response: 
[    pdpInternal][DEBUG]    response: [ALLOW]
[    pdpInternal][DEBUG]    delay:    [0]
[    pdpInternal][DEBUG]    modifiers:[0]
[       pefEvent][TRACE] Freeing event referencing action=[action1]
[       pefParam][TRACE] Freeing event parameter [val1]
[       pefParam][TRACE] Freeing parameter [val1][value1][type:0]
[       pefEvent][DEBUG] Event id=[0] deallocated
[  testOperators][TRACE] response=[<notifyEventResponse>
    <authorizationAction>
      <allow/>
    </authorizationAction>
  </notifyEventResponse>]
[      mechanism][WARN ] ////////////////////////////////////////////////////////////////////////////////////////////////////////////
[      mechanism][DEBUG] mechanismUpdate - [testDuring] Null-Event updating 5. timestep at interval of 3000000 us
[      condition][TRACE] conditionUpdate - updating with event=[00000000]
[        oslEval][TRACE]  [testDuring] - evaluating EVENT    node => 1
[        oslEval][TRACE]  [testDuring] - evaluating DURING   history available=[true]
[        oslEval][TRACE] [DURING] circArray=[0 0 0 ]
[        oslEval][TRACE] [DURING] Setting position [2] to [1]
[        oslEval][TRACE] [DURING] circArray=[0 0 1 ]
[        oslEval][TRACE]  [testDuring] - evaluating DURING   node => 0
[      condition][DEBUG]  [testDuring] - condition value=[false]
[      mechanism][INFO ] mechanismUpdate - [testDuring] event=[0] timestep=[5] value=[false]
[      mechanism][TRACE] mechanismUpdate - [testDuring] -----------------------------------------------------------
[            pdp][TRACE] receiving event (1): [<event isTry="true" action="action1" index="ALL"><parameter name="val1" value="value1"/></event>]
[actionDescStore][TRACE] Action description [action1] found in store
[       pefEvent][TRACE] Creating event action=[action1] index=[ongoing] is_try=[true]
[       pefEvent][TRACE] Event id=[1] [action1][ongoing][true] created
[       pefEvent][DEBUG] no parameter type given for event; defaulting to PARAM_SRING!
[         action][TRACE] Param description [val1] found
[       pefParam][TRACE] Adding parameter [val1][value1][type:0] to event
[       pefEvent][DEBUG] Successfully parsed XML and created event referencing action "action1"
[            pdp][DEBUG] Searching for subscribed condition nodes for event=[action1]; subscribed nodes=[1]
[            pdp][DEBUG] conditionTriggerEvent - checking condition eventMatch=[action1] for levent=[action1]
[  pefEventMatch][DEBUG] Matching event [1]
[  pefEventMatch][TRACE] Event index matches operator [ongoing]~=[ongoing]
[  pefEventMatch][TRACE] Event is_try matches operator [true]=[true]
[  pefEventMatch][TRACE] Event action matches [action1]=[action1]
[  pefParamMatch][DEBUG] matchEventParameters - comparing trigger event (nParam=[1]) with desired event(nParam=[1])
[  pefParamMatch][DEBUG] param [val1]: param value for matching, calling specific matching function...
[  pefParamMatch][DEBUG] Matching parameter using common compare
[       pefParam][DEBUG] Event parameter [val1] found
[  pefParamMatch][DEBUG] Parameter [val1] match
[  pefEventMatch][DEBUG] Event matches operator
[            pdp][TRACE]  evaluated EVENT    node => 1
[            pdp][DEBUG] Searching for triggered mechanism for event=[action1]; subscribed mechanisms=[1]
[            pdp][DEBUG] mechanismTriggerEvent - checking mech=[testDuring] for levent=[action1]
[  pefEventMatch][DEBUG] Matching event [1]
[  pefEventMatch][TRACE] Event index matches operator [ongoing]~=[ongoing]
[  pefEventMatch][TRACE] Event is_try matches operator [true]=[true]
[  pefEventMatch][TRACE] Event action matches [action1]=[action1]
[  pefParamMatch][DEBUG] matchEventParameters - comparing trigger event (nParam=[1]) with desired event(nParam=[1])
[  pefParamMatch][DEBUG] param [val1]: param value for matching, calling specific matching function...
[  pefParamMatch][DEBUG] Matching parameter using common compare
[       pefParam][DEBUG] Event parameter [val1] found
[  pefParamMatch][DEBUG] Parameter [val1] match
[  pefEventMatch][DEBUG] Event matches operator
[            pdp][INFO ] mechanismTriggerEvent - Found matching mechanism -> mechanism_name=[testDuring]
[authorizationAc][DEBUG] Successfully allocated new mechanism_action (response: 1, delay: 0)
[      condition][TRACE] conditionUpdate - updating with event=[00e60740]
[  pefEventMatch][DEBUG] Matching event [1]
[  pefEventMatch][TRACE] Event index matches operator [ongoing]~=[ongoing]
[  pefEventMatch][TRACE] Event is_try matches operator [true]=[true]
[  pefEventMatch][TRACE] Event action matches [action1]=[action1]
[  pefParamMatch][DEBUG] matchEventParameters - comparing trigger event (nParam=[1]) with desired event(nParam=[1])
[  pefParamMatch][DEBUG] param [val1]: param value for matching, calling specific matching function...
[  pefParamMatch][DEBUG] Matching parameter using common compare
[       pefParam][DEBUG] Event parameter [val1] found
[  pefParamMatch][DEBUG] Parameter [val1] match
[  pefEventMatch][DEBUG] Event matches operator
[        oslEval][TRACE]  [testDuring] - evaluating EVENT    node => 1
[        oslEval][TRACE]  [testDuring] - evaluating DURING   history available=[true]
[        oslEval][TRACE] [DURING] circArray=[0 0 1 ]
[        oslEval][TRACE]  [testDuring] - evaluating DURING   node => 0
[      condition][DEBUG]  [testDuring] - condition value=[false]
[            pdp][TRACE] condition NOT satisfied, doing nothing...
[    pdpInternal][DEBUG] notifyEvent response: 
[    pdpInternal][DEBUG]    response: [ALLOW]
[    pdpInternal][DEBUG]    delay:    [0]
[    pdpInternal][DEBUG]    modifiers:[0]
[       pefEvent][TRACE] Freeing event referencing action=[action1]
[       pefParam][TRACE] Freeing event parameter [val1]
[       pefParam][TRACE] Freeing parameter [val1][value1][type:0]
[       pefEvent][DEBUG] Event id=[1] deallocated
[  testOperators][TRACE] response=[<notifyEventResponse>
    <authorizationAction>
      <allow/>
    </authorizationAction>
  </notifyEventResponse>]
[      mechanism][WARN ] ////////////////////////////////////////////////////////////////////////////////////////////////////////////
[      mechanism][DEBUG] mechanismUpdate - [testDuring] Null-Event updating 6. timestep at interval of 3000000 us
[      condition][TRACE] conditionUpdate - updating with event=[00000000]
[        oslEval][TRACE]  [testDuring] - evaluating EVENT    node => 1
[        oslEval][TRACE]  [testDuring] - evaluating DURING   history available=[true]
[        oslEval][TRACE] [DURING] circArray=[0 0 1 ]
[        oslEval][TRACE] [DURING] Setting position [0] to [1]
[        oslEval][TRACE] [DURING] circArray=[1 0 1 ]
[        oslEval][TRACE]  [testDuring] - evaluating DURING   node => 0
[      condition][DEBUG]  [testDuring] - condition value=[false]
[      mechanism][INFO ] mechanismUpdate - [testDuring] event=[0] timestep=[6] value=[false]
[      mechanism][TRACE] mechanismUpdate - [testDuring] -----------------------------------------------------------
[            pdp][TRACE] receiving event (2): [<event isTry="true" action="action1" index="ALL"><parameter name="val1" value="value1"/></event>]
[actionDescStore][TRACE] Action description [action1] found in store
[       pefEvent][TRACE] Creating event action=[action1] index=[ongoing] is_try=[true]
[       pefEvent][TRACE] Event id=[2] [action1][ongoing][true] created
[       pefEvent][DEBUG] no parameter type given for event; defaulting to PARAM_SRING!
[         action][TRACE] Param description [val1] found
[       pefParam][TRACE] Adding parameter [val1][value1][type:0] to event
[       pefEvent][DEBUG] Successfully parsed XML and created event referencing action "action1"
[            pdp][DEBUG] Searching for subscribed condition nodes for event=[action1]; subscribed nodes=[1]
[            pdp][DEBUG] conditionTriggerEvent - checking condition eventMatch=[action1] for levent=[action1]
[  pefEventMatch][DEBUG] Matching event [2]
[  pefEventMatch][TRACE] Event index matches operator [ongoing]~=[ongoing]
[  pefEventMatch][TRACE] Event is_try matches operator [true]=[true]
[  pefEventMatch][TRACE] Event action matches [action1]=[action1]
[  pefParamMatch][DEBUG] matchEventParameters - comparing trigger event (nParam=[1]) with desired event(nParam=[1])
[  pefParamMatch][DEBUG] param [val1]: param value for matching, calling specific matching function...
[  pefParamMatch][DEBUG] Matching parameter using common compare
[       pefParam][DEBUG] Event parameter [val1] found
[  pefParamMatch][DEBUG] Parameter [val1] match
[  pefEventMatch][DEBUG] Event matches operator
[            pdp][TRACE]  evaluated EVENT    node => 1
[            pdp][DEBUG] Searching for triggered mechanism for event=[action1]; subscribed mechanisms=[1]
[            pdp][DEBUG] mechanismTriggerEvent - checking mech=[testDuring] for levent=[action1]
[  pefEventMatch][DEBUG] Matching event [2]
[  pefEventMatch][TRACE] Event index matches operator [ongoing]~=[ongoing]
[  pefEventMatch][TRACE] Event is_try matches operator [true]=[true]
[  pefEventMatch][TRACE] Event action matches [action1]=[action1]
[  pefParamMatch][DEBUG] matchEventParameters - comparing trigger event (nParam=[1]) with desired event(nParam=[1])
[  pefParamMatch][DEBUG] param [val1]: param value for matching, calling specific matching function...
[  pefParamMatch][DEBUG] Matching parameter using common compare
[       pefParam][DEBUG] Event parameter [val1] found
[  pefParamMatch][DEBUG] Parameter [val1] match
[  pefEventMatch][DEBUG] Event matches operator
[            pdp][INFO ] mechanismTriggerEvent - Found matching mechanism -> mechanism_name=[testDuring]
[authorizationAc][DEBUG] Successfully allocated new mechanism_action (response: 1, delay: 0)
[      condition][TRACE] conditionUpdate - updating with event=[00e60740]
[  pefEventMatch][DEBUG] Matching event [2]
[  pefEventMatch][TRACE] Event index matches operator [ongoing]~=[ongoing]
[  pefEventMatch][TRACE] Event is_try matches operator [true]=[true]
[  pefEventMatch][TRACE] Event action matches [action1]=[action1]
[  pefParamMatch][DEBUG] matchEventParameters - comparing trigger event (nParam=[1]) with desired event(nParam=[1])
[  pefParamMatch][DEBUG] param [val1]: param value for matching, calling specific matching function...
[  pefParamMatch][DEBUG] Matching parameter using common compare
[       pefParam][DEBUG] Event parameter [val1] found
[  pefParamMatch][DEBUG] Parameter [val1] match
[  pefEventMatch][DEBUG] Event matches operator
[        oslEval][TRACE]  [testDuring] - evaluating EVENT    node => 1
[        oslEval][TRACE]  [testDuring] - evaluating DURING   history available=[true]
[        oslEval][TRACE] [DURING] circArray=[1 0 1 ]
[        oslEval][TRACE]  [testDuring] - evaluating DURING   node => 0
[      condition][DEBUG]  [testDuring] - condition value=[false]
[            pdp][TRACE] condition NOT satisfied, doing nothing...
[    pdpInternal][DEBUG] notifyEvent response: 
[    pdpInternal][DEBUG]    response: [ALLOW]
[    pdpInternal][DEBUG]    delay:    [0]
[    pdpInternal][DEBUG]    modifiers:[0]
[       pefEvent][TRACE] Freeing event referencing action=[action1]
[       pefParam][TRACE] Freeing event parameter [val1]
[       pefParam][TRACE] Freeing parameter [val1][value1][type:0]
[       pefEvent][DEBUG] Event id=[2] deallocated
[  testOperators][TRACE] response=[<notifyEventResponse>
    <authorizationAction>
      <allow/>
    </authorizationAction>
  </notifyEventResponse>]
[      mechanism][WARN ] ////////////////////////////////////////////////////////////////////////////////////////////////////////////
[      mechanism][DEBUG] mechanismUpdate - [testDuring] Null-Event updating 7. timestep at interval of 3000000 us
[      condition][TRACE] conditionUpdate - updating with event=[00000000]
[        oslEval][TRACE]  [testDuring] - evaluating EVENT    node => 1
[        oslEval][TRACE]  [testDuring] - evaluating DURING   history available=[true]
[        oslEval][TRACE] [DURING] circArray=[1 0 1 ]
[        oslEval][TRACE] [DURING] Setting position [1] to [1]
[        oslEval][TRACE] [DURING] circArray=[1 1 1 ]
[        oslEval][TRACE]  [testDuring] - evaluating DURING   node => 1
[      condition][DEBUG]  [testDuring] - condition value=[true]
[      mechanism][INFO ] mechanismUpdate - [testDuring] event=[0] timestep=[7] value=[true]
[      mechanism][TRACE] mechanismUpdate - [testDuring] -----------------------------------------------------------
[            pdp][TRACE] receiving event (3): [<event isTry="true" action="action1" index="ALL"><parameter name="val1" value="value1"/></event>]
[actionDescStore][TRACE] Action description [action1] found in store
[       pefEvent][TRACE] Creating event action=[action1] index=[ongoing] is_try=[true]
[       pefEvent][TRACE] Event id=[3] [action1][ongoing][true] created
[       pefEvent][DEBUG] no parameter type given for event; defaulting to PARAM_SRING!
[         action][TRACE] Param description [val1] found
[       pefParam][TRACE] Adding parameter [val1][value1][type:0] to event
[       pefEvent][DEBUG] Successfully parsed XML and created event referencing action "action1"
[            pdp][DEBUG] Searching for subscribed condition nodes for event=[action1]; subscribed nodes=[1]
[            pdp][DEBUG] conditionTriggerEvent - checking condition eventMatch=[action1] for levent=[action1]
[  pefEventMatch][DEBUG] Matching event [3]
[  pefEventMatch][TRACE] Event index matches operator [ongoing]~=[ongoing]
[  pefEventMatch][TRACE] Event is_try matches operator [true]=[true]
[  pefEventMatch][TRACE] Event action matches [action1]=[action1]
[  pefParamMatch][DEBUG] matchEventParameters - comparing trigger event (nParam=[1]) with desired event(nParam=[1])
[  pefParamMatch][DEBUG] param [val1]: param value for matching, calling specific matching function...
[  pefParamMatch][DEBUG] Matching parameter using common compare
[       pefParam][DEBUG] Event parameter [val1] found
[  pefParamMatch][DEBUG] Parameter [val1] match
[  pefEventMatch][DEBUG] Event matches operator
[            pdp][TRACE]  evaluated EVENT    node => 1
[            pdp][DEBUG] Searching for triggered mechanism for event=[action1]; subscribed mechanisms=[1]
[            pdp][DEBUG] mechanismTriggerEvent - checking mech=[testDuring] for levent=[action1]
[  pefEventMatch][DEBUG] Matching event [3]
[  pefEventMatch][TRACE] Event index matches operator [ongoing]~=[ongoing]
[  pefEventMatch][TRACE] Event is_try matches operator [true]=[true]
[  pefEventMatch][TRACE] Event action matches [action1]=[action1]
[  pefParamMatch][DEBUG] matchEventParameters - comparing trigger event (nParam=[1]) with desired event(nParam=[1])
[  pefParamMatch][DEBUG] param [val1]: param value for matching, calling specific matching function...
[  pefParamMatch][DEBUG] Matching parameter using common compare
[       pefParam][DEBUG] Event parameter [val1] found
[  pefParamMatch][DEBUG] Parameter [val1] match
[  pefEventMatch][DEBUG] Event matches operator
[            pdp][INFO ] mechanismTriggerEvent - Found matching mechanism -> mechanism_name=[testDuring]
[authorizationAc][DEBUG] Successfully allocated new mechanism_action (response: 1, delay: 0)
[      condition][TRACE] conditionUpdate - updating with event=[00e60740]
[  pefEventMatch][DEBUG] Matching event [3]
[  pefEventMatch][TRACE] Event index matches operator [ongoing]~=[ongoing]
[  pefEventMatch][TRACE] Event is_try matches operator [true]=[true]
[  pefEventMatch][TRACE] Event action matches [action1]=[action1]
[  pefParamMatch][DEBUG] matchEventParameters - comparing trigger event (nParam=[1]) with desired event(nParam=[1])
[  pefParamMatch][DEBUG] param [val1]: param value for matching, calling specific matching function...
[  pefParamMatch][DEBUG] Matching parameter using common compare
[       pefParam][DEBUG] Event parameter [val1] found
[  pefParamMatch][DEBUG] Parameter [val1] match
[  pefEventMatch][DEBUG] Event matches operator
[        oslEval][TRACE]  [testDuring] - evaluating EVENT    node => 1
[        oslEval][TRACE]  [testDuring] - evaluating DURING   history available=[true]
[        oslEval][TRACE] [DURING] circArray=[1 1 1 ]
[        oslEval][TRACE]  [testDuring] - evaluating DURING   node => 1
[      condition][DEBUG]  [testDuring] - condition value=[true]
[    pdpInternal][DEBUG] Adding mechanism=[testDuring] to response
[    pdpInternal][TRACE] updating ACTION_ALLOW to ACTION_INHIBIT
[    pdpInternal][DEBUG] notifyEvent response: 
[    pdpInternal][DEBUG]    response: [INHIBIT]
[    pdpInternal][DEBUG]    delay:    [0]
[    pdpInternal][DEBUG]    modifiers:[0]
[       pefEvent][TRACE] Freeing event referencing action=[action1]
[       pefParam][TRACE] Freeing event parameter [val1]
[       pefParam][TRACE] Freeing parameter [val1][value1][type:0]
[       pefEvent][DEBUG] Event id=[3] deallocated
[  testOperators][TRACE] response=[<notifyEventResponse>
    <authorizationAction>
      <inhibit/>
    </authorizationAction>
  </notifyEventResponse>]
[      mechanism][WARN ] ////////////////////////////////////////////////////////////////////////////////////////////////////////////
[      mechanism][DEBUG] mechanismUpdate - [testDuring] Null-Event updating 8. timestep at interval of 3000000 us
[      condition][TRACE] conditionUpdate - updating with event=[00000000]
[        oslEval][TRACE]  [testDuring] - evaluating EVENT    node => 1
[        oslEval][TRACE]  [testDuring] - evaluating DURING   history available=[true]
[        oslEval][TRACE] [DURING] circArray=[1 1 1 ]
[        oslEval][TRACE] [DURING] Setting position [2] to [1]
[        oslEval][TRACE] [DURING] circArray=[1 1 1 ]
[        oslEval][TRACE]  [testDuring] - evaluating DURING   node => 1
[      condition][DEBUG]  [testDuring] - condition value=[true]
[      mechanism][INFO ] mechanismUpdate - [testDuring] event=[0] timestep=[8] value=[true]
[      mechanism][TRACE] mechanismUpdate - [testDuring] -----------------------------------------------------------
[            pdp][TRACE] receiving event (4): [<event isTry="true" action="action1" index="ALL"><parameter name="val1" value="value1"/></event>]
[actionDescStore][TRACE] Action description [action1] found in store
[       pefEvent][TRACE] Creating event action=[action1] index=[ongoing] is_try=[true]
[       pefEvent][TRACE] Event id=[4] [action1][ongoing][true] created
[       pefEvent][DEBUG] no parameter type given for event; defaulting to PARAM_SRING!
[         action][TRACE] Param description [val1] found
[       pefParam][TRACE] Adding parameter [val1][value1][type:0] to event
[       pefEvent][DEBUG] Successfully parsed XML and created event referencing action "action1"
[            pdp][DEBUG] Searching for subscribed condition nodes for event=[action1]; subscribed nodes=[1]
[            pdp][DEBUG] conditionTriggerEvent - checking condition eventMatch=[action1] for levent=[action1]
[  pefEventMatch][DEBUG] Matching event [4]
[  pefEventMatch][TRACE] Event index matches operator [ongoing]~=[ongoing]
[  pefEventMatch][TRACE] Event is_try matches operator [true]=[true]
[  pefEventMatch][TRACE] Event action matches [action1]=[action1]
[  pefParamMatch][DEBUG] matchEventParameters - comparing trigger event (nParam=[1]) with desired event(nParam=[1])
[  pefParamMatch][DEBUG] param [val1]: param value for matching, calling specific matching function...
[  pefParamMatch][DEBUG] Matching parameter using common compare
[       pefParam][DEBUG] Event parameter [val1] found
[  pefParamMatch][DEBUG] Parameter [val1] match
[  pefEventMatch][DEBUG] Event matches operator
[            pdp][TRACE]  evaluated EVENT    node => 1
[            pdp][DEBUG] Searching for triggered mechanism for event=[action1]; subscribed mechanisms=[1]
[            pdp][DEBUG] mechanismTriggerEvent - checking mech=[testDuring] for levent=[action1]
[  pefEventMatch][DEBUG] Matching event [4]
[  pefEventMatch][TRACE] Event index matches operator [ongoing]~=[ongoing]
[  pefEventMatch][TRACE] Event is_try matches operator [true]=[true]
[  pefEventMatch][TRACE] Event action matches [action1]=[action1]
[  pefParamMatch][DEBUG] matchEventParameters - comparing trigger event (nParam=[1]) with desired event(nParam=[1])
[  pefParamMatch][DEBUG] param [val1]: param value for matching, calling specific matching function...
[  pefParamMatch][DEBUG] Matching parameter using common compare
[       pefParam][DEBUG] Event parameter [val1] found
[  pefParamMatch][DEBUG] Parameter [val1] match
[  pefEventMatch][DEBUG] Event matches operator
[            pdp][INFO ] mechanismTriggerEvent - Found matching mechanism -> mechanism_name=[testDuring]
[authorizationAc][DEBUG] Successfully allocated new mechanism_action (response: 1, delay: 0)
[      condition][TRACE] conditionUpdate - updating with event=[00e60740]
[  pefEventMatch][DEBUG] Matching event [4]
[  pefEventMatch][TRACE] Event index matches operator [ongoing]~=[ongoing]
[  pefEventMatch][TRACE] Event is_try matches operator [true]=[true]
[  pefEventMatch][TRACE] Event action matches [action1]=[action1]
[  pefParamMatch][DEBUG] matchEventParameters - comparing trigger event (nParam=[1]) with desired event(nParam=[1])
[  pefParamMatch][DEBUG] param [val1]: param value for matching, calling specific matching function...
[  pefParamMatch][DEBUG] Matching parameter using common compare
[       pefParam][DEBUG] Event parameter [val1] found
[  pefParamMatch][DEBUG] Parameter [val1] match
[  pefEventMatch][DEBUG] Event matches operator
[        oslEval][TRACE]  [testDuring] - evaluating EVENT    node => 1
[        oslEval][TRACE]  [testDuring] - evaluating DURING   history available=[true]
[        oslEval][TRACE] [DURING] circArray=[1 1 1 ]
[        oslEval][TRACE]  [testDuring] - evaluating DURING   node => 1
[      condition][DEBUG]  [testDuring] - condition value=[true]
[    pdpInternal][DEBUG] Adding mechanism=[testDuring] to response
[    pdpInternal][TRACE] updating ACTION_ALLOW to ACTION_INHIBIT
[    pdpInternal][DEBUG] notifyEvent response: 
[    pdpInternal][DEBUG]    response: [INHIBIT]
[    pdpInternal][DEBUG]    delay:    [0]
[    pdpInternal][DEBUG]    modifiers:[0]
[       pefEvent][TRACE] Freeing event referencing action=[action1]
[       pefParam][TRACE] Freeing event parameter [val1]
[       pefParam][TRACE] Freeing parameter [val1][value1][type:0]
[       pefEvent][DEBUG] Event id=[4] deallocated
[  testOperators][TRACE] response=[<notifyEventResponse>
    <authorizationAction>
      <inhibit/>
    </authorizationAction>
  </notifyEventResponse>]
[      mechanism][WARN ] ////////////////////////////////////////////////////////////////////////////////////////////////////////////
[      mechanism][DEBUG] mechanismUpdate - [testDuring] Null-Event updating 9. timestep at interval of 3000000 us
[      condition][TRACE] conditionUpdate - updating with event=[00000000]
[        oslEval][TRACE]  [testDuring] - evaluating EVENT    node => 1
[        oslEval][TRACE]  [testDuring] - evaluating DURING   history available=[true]
[        oslEval][TRACE] [DURING] circArray=[1 1 1 ]
[        oslEval][TRACE] [DURING] Setting position [0] to [1]
[        oslEval][TRACE] [DURING] circArray=[1 1 1 ]
[        oslEval][TRACE]  [testDuring] - evaluating DURING   node => 1
[      condition][DEBUG]  [testDuring] - condition value=[true]
[      mechanism][INFO ] mechanismUpdate - [testDuring] event=[0] timestep=[9] value=[true]
[      mechanism][TRACE] mechanismUpdate - [testDuring] -----------------------------------------------------------
[      mechanism][WARN ] ////////////////////////////////////////////////////////////////////////////////////////////////////////////
[      mechanism][DEBUG] mechanismUpdate - [testDuring] Null-Event updating 10. timestep at interval of 3000000 us
[      condition][TRACE] conditionUpdate - updating with event=[00000000]
[        oslEval][TRACE]  [testDuring] - evaluating EVENT    node => 0
[        oslEval][TRACE]  [testDuring] - evaluating DURING   history available=[true]
[        oslEval][TRACE] [DURING] circArray=[1 1 1 ]
[        oslEval][TRACE] [DURING] Setting position [1] to [0]
[        oslEval][TRACE] [DURING] circArray=[1 0 1 ]
[        oslEval][TRACE]  [testDuring] - evaluating DURING   node => 0
[      condition][DEBUG]  [testDuring] - condition value=[false]
[      mechanism][INFO ] mechanismUpdate - [testDuring] event=[0] timestep=[10] value=[false]
[      mechanism][TRACE] mechanismUpdate - [testDuring] -----------------------------------------------------------
[      mechanism][WARN ] ////////////////////////////////////////////////////////////////////////////////////////////////////////////
[      mechanism][DEBUG] mechanismUpdate - [testDuring] Null-Event updating 11. timestep at interval of 3000000 us
[      condition][TRACE] conditionUpdate - updating with event=[00000000]
[        oslEval][TRACE]  [testDuring] - evaluating EVENT    node => 0
[        oslEval][TRACE]  [testDuring] - evaluating DURING   history available=[true]
[        oslEval][TRACE] [DURING] circArray=[1 0 1 ]
[        oslEval][TRACE] [DURING] Setting position [2] to [0]
[        oslEval][TRACE] [DURING] circArray=[1 0 0 ]
[        oslEval][TRACE]  [testDuring] - evaluating DURING   node => 0
[      condition][DEBUG]  [testDuring] - condition value=[false]
[      mechanism][INFO ] mechanismUpdate - [testDuring] event=[0] timestep=[11] value=[false]
[      mechanism][TRACE] mechanismUpdate - [testDuring] -----------------------------------------------------------
[      mechanism][WARN ] ////////////////////////////////////////////////////////////////////////////////////////////////////////////
[      mechanism][DEBUG] mechanismUpdate - [testDuring] Null-Event updating 12. timestep at interval of 3000000 us
[      condition][TRACE] conditionUpdate - updating with event=[00000000]
[        oslEval][TRACE]  [testDuring] - evaluating EVENT    node => 0
[        oslEval][TRACE]  [testDuring] - evaluating DURING   history available=[true]
[        oslEval][TRACE] [DURING] circArray=[1 0 0 ]
[        oslEval][TRACE] [DURING] Setting position [0] to [0]
[        oslEval][TRACE] [DURING] circArray=[0 0 0 ]
[        oslEval][TRACE]  [testDuring] - evaluating DURING   node => 0
[      condition][DEBUG]  [testDuring] - condition value=[false]
[      mechanism][INFO ] mechanismUpdate - [testDuring] event=[0] timestep=[12] value=[false]
[      mechanism][TRACE] mechanismUpdate - [testDuring] -----------------------------------------------------------
[      mechanism][WARN ] ////////////////////////////////////////////////////////////////////////////////////////////////////////////
[      mechanism][DEBUG] mechanismUpdate - [testDuring] Null-Event updating 13. timestep at interval of 3000000 us
[      condition][TRACE] conditionUpdate - updating with event=[00000000]
[        oslEval][TRACE]  [testDuring] - evaluating EVENT    node => 0
[        oslEval][TRACE]  [testDuring] - evaluating DURING   history available=[true]
[        oslEval][TRACE] [DURING] circArray=[0 0 0 ]
[        oslEval][TRACE] [DURING] Setting position [1] to [0]
[        oslEval][TRACE] [DURING] circArray=[0 0 0 ]
[        oslEval][TRACE]  [testDuring] - evaluating DURING   node => 0
[      condition][DEBUG]  [testDuring] - condition value=[false]
[      mechanism][INFO ] mechanismUpdate - [testDuring] event=[0] timestep=[13] value=[false]
[      mechanism][TRACE] mechanismUpdate - [testDuring] -----------------------------------------------------------
[      mechanism][WARN ] ////////////////////////////////////////////////////////////////////////////////////////////////////////////
[      mechanism][DEBUG] mechanismUpdate - [testDuring] Null-Event updating 14. timestep at interval of 3000000 us
[      condition][TRACE] conditionUpdate - updating with event=[00000000]
[        oslEval][TRACE]  [testDuring] - evaluating EVENT    node => 0
[        oslEval][TRACE]  [testDuring] - evaluating DURING   history available=[true]
[        oslEval][TRACE] [DURING] circArray=[0 0 0 ]
[        oslEval][TRACE] [DURING] Setting position [2] to [0]
[        oslEval][TRACE] [DURING] circArray=[0 0 0 ]
[        oslEval][TRACE]  [testDuring] - evaluating DURING   node => 0
[      condition][DEBUG]  [testDuring] - condition value=[false]
[      mechanism][INFO ] mechanismUpdate - [testDuring] event=[0] timestep=[14] value=[false]
[      mechanism][TRACE] mechanismUpdate - [testDuring] -----------------------------------------------------------
[            pdp][DEBUG] trying to deallocate pdpMutex
[    threadUtils][TRACE] Deallocating mechanism mutex [pdpMutex]
[    threadUtils][DEBUG] Mechanism mutex successfully deallocated.
Logger mutex successfully deallocated.
Cleaning up libxml2 related memory
pdpDestructor finished
