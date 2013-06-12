package de.fraunhofer.iese.pef.pip;

public interface IPolicyInformationPoint
{
  // value1 -> representation;  value2 -> data;  returns boolean flag whether representation refines data
  // value1 -> name of context; value2 -> param; returns boolean flag whether context is satisfied
  int eval(String value1, String value2);
  
  // method -> deployContext or initialRepresentation
  String init(String method, String param);
}
