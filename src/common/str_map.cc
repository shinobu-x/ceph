// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*- 
// vim: ts=8 sw=2 smarttab
/*
 * Ceph - scalable distributed file system
 *
 * Copyright (C) 2013 Cloudwatt <libre.licensing@cloudwatt.com>
 *
 * Author: Loic Dachary <loic@dachary.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 * 
 */

#include <errno.h>

#include "include/str_map.h"
#include "include/str_list.h"

#include "json_spirit/json_spirit.h"


int get_json_str_map(
    const std::string &str,
    std::ostream &ss,
    std::map<std::string, std::string> *str_map,
    bool fallback_to_plain)
{
  json_spirit::mValue json;
  try {
    // try json parsing first

    json_spirit::read_or_throw(str, json);

    if (json.type() != json_spirit::obj_type) {
      ss << str << " must be a JSON object but is of type "
	 << json.type() << " instead";
      return -EINVAL;
    }

    json_spirit::mObject o = json.get_obj();

    for (std::map<std::string, json_spirit::mValue>::iterator i = o.begin();
	 i != o.end();
	 ++i) {
      (*str_map)[i->first] = i->second.get_str();
    }
  } catch (json_spirit::Error_position &e) {
    if (fallback_to_plain) {
      // fallback to key=value format
      get_str_map(str, str_map, "\t\n ");
    } else {
      return -EINVAL;
    }
  }
  return 0;
}
std::string trim(const std::string& str) {
  size_t start = 0;
  size_t end = str.size() - 1;
  while (isspace(str[start]) != 0 && start <= end) {
    ++start;
  }
  while (isspace(str[end]) != 0 && start <= end) {
    --end;
  }
  if (start <= end) {
    return str.substr(start, end - start + 1);
  }
  return std::string();
}

int get_str_map(
    const std::string &str,
    std::map<std::string, std::string> *str_map,
    const char *delims)
{
  std::list<std::string> pairs;
  get_str_list(str, delims, pairs);
  for (std::list<std::string>::iterator i = pairs.begin(); 
       i != pairs.end(); ++i) {
    size_t equal = i->find('=');
    if (equal == std::string::npos)
      (*str_map)[*i] = std::string();
    else {
      const std::string key = trim(i->substr(0, equal));
      equal++;
      const std::string value = trim(i->substr(equal));
      (*str_map)[key] = value;
    }
  }
  return 0;
}

std::string get_str_map_value(
    const std::map<std::string, std::string> &str_map,
    const std::string &key,
    const std::string *def_val)
{
  std::map<std::string, std::string>::const_iterator p = str_map.find(key);

  // key exists in str_map
  if (p != str_map.end()) {
    // but value is empty
    if (p->second.empty())
      return p->first;
    // and value is not empty
    return p->second;
  }

  // key DNE in str_map and def_val was specified
  if (def_val != NULL)
    return *def_val;

  // key DNE in str_map, no def_val was specified
  return std::string();
}

std::string get_str_map_key(
    const std::map<std::string, std::string> &str_map,
    const std::string &key,
    const std::string *fallback_key)
{
  std::map<std::string, std::string>::const_iterator p = str_map.find(key);
  if (p != str_map.end())
    return p->second;

  if (fallback_key != NULL) {
    p = str_map.find(*fallback_key);
    if (p != str_map.end())
      return p->second;
  }
  return std::string();
}

// This function's only purpose is to check whether a given map has only
// ONE key with an empty value (which would mean that 'get_str_map()' read
// a map in the form of 'VALUE', without any KEY/VALUE pairs) and, in such
// event, to assign said 'VALUE' to a given 'def_key', such that we end up
// with a map of the form "m = { 'def_key' : 'VALUE' }" instead of the
// original "m = { 'VALUE' : '' }".
int get_conf_str_map_helper(
    const std::string &str,
    std::ostringstream &oss,
    std::map<std::string, std::string> *m,
    const std::string &def_key)
{
  int r = get_str_map(str, m);

  if (r < 0) {
    return r;
  }

  if (r >= 0 && m->size() == 1) {
    std::map<std::string, std::string>::iterator p = m->begin();
    if (p->second.empty()) {
      std::string s = p->first;
      m->erase(s);
      (*m)[def_key] = s;
    }
  }
  return r;
}
