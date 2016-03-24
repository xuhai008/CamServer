-- event_conference.lua
-- author : Max.Chiu
-- date : 2016/03/24
-- freeswitch会议事件监听脚本

api = freeswitch.API();

freeswitch.consoleLog("CONSOLE", "# 会议事件监听脚本->开始");
-- 获取事件类型
fun = event:getHeader("Event-Calling-Function");
freeswitch.consoleLog("CONSOLE", "# 会议事件监听脚本->事件:[" .. fun .. "]");
freeswitch.consoleLog("CONSOLE", "# 会议事件监听脚本->event:\n" .. event:serialize("json"));
  
-- 处理会议增加成员
if fun == "conference_member_add" then
  conference_name = event:getHeader("Conference-Name");
  member_type = event:getHeader("Member-Type");
  member_id = event:getHeader("Member-ID");
  
  -- 静音, 静视频
  api:execute("conference", conference_name .. " mute " .. member_id);
  api:execute("conference", conference_name .. " vmute " .. member_id);
  api:execute("conference", conference_name .. " relate " .. member_id .. member_id .. "clear");
  api:execute("conference", conference_name .. " relate " .. member_id .. member_id .. "sendvideo");
    
  -- 根据用户类型设置权限
  if member_type == "moderator" then
    freeswitch.consoleLog("CONSOLE", "# 会议事件监听脚本->事件:增加主持人[member_id:" .. member_id .. "]到会议室[" .. conference_name .. "]");
    api:execute("conference", conference_name .. " vid-floor " .. member_id .. " force");
  else
    freeswitch.consoleLog("CONSOLE", "# 会议事件监听脚本->事件:增加成员[member_id:" .. member_id .. "]到会议室[" .. conference_name .. "]");
  end
  
end -- 处理会议增加成员完成

freeswitch.consoleLog("CONSOLE", "# 会议事件监听脚本->结束");