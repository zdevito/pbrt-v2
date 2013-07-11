local ffi = require("ffi")

local function resize(self,minsize)
	local N = self.allocsize
	if minsize > N then
		local newN = math.max(8,N)
		while newN < minsize do
			newN = newN*2
		end
		local ctor = self.ctor
		local elemsize = self.elemsize
		local newdata = ctor(newN)
		local olddata = rawget(self,"data")
		if olddata then
			ffi.copy(newdata,olddata,N*elemsize)
		end
		ffi.fill(newdata + N, (newN - N)*elemsize)
		rawset(self, "data", newdata)
		self.allocsize = newN
	end
	self.size = math.max(self.size,minsize)
end
local function paramindex(self,idx)
	idx = tonumber(idx) or error("idx not number? "..tostring(idx))
	resize(self,idx+1)
	return rawget(self,"data")[idx]
end

local function paramnewindex(self,idx,value)
	idx = tonumber(idx) or error("idx not number? "..tostring(idx))
	resize(self,idx+1)
	rawget(self,"data")[idx] = value
end

local paramtable = { __index = paramindex, __newindex = paramnewindex }
local function newparam(typ,N)
	local ctor = ffi.typeof(typ.."[?]")
	local elemsize = ffi.sizeof(typ)
	assert(ctor)
	local self = setmetatable({ size = 0, allocsize = 0, elemsize = elemsize, ctor = ctor },paramtable)
	if N and type(N) == "number" then
		resize(self,N)
	elseif N and type(N) == "table" then
		local sz = #N
		self.allocsize,self.size = sz,sz
		local nd = self.ctor(sz,N)
		rawset(self,"data",nd)
	end
	return self
end

ffi.cdef [[
typedef float float3[3];
]]
--WARNING ORDER OF ENTRIES MUST MATCH ENUM ORDER IN pbrtlua.cpp
local kinds = { { "Float", "float" }, { "Int", "int" }, { "Bool", "bool" }, { "Point", "float3" }, { "Vector", "float3" }, { "Normal", "float3" },
                { "RGBSpectrum", "float3" }, { "XYZSpectrum", "float3" }, { "BlackbodySpectrum", "float" }, { "SampledSpectrum", "float" } }

for i,k in ipairs(kinds) do
    local nm,typ = unpack(k)
    _G[nm] = function(N)
        local p = newparam(typ,N)
        rawset(p,"kind",i-1)
        return p
    end
end
local stringkind,texturekind = 11,12
function String(N)
    local self = { kind = stringkind }
    if type(N) == "table" then
        for i,s in ipairs(N) do
            self[i] = tostring(s)
        end
    end
    return self
end
function Texture(tex)
    return { kind = texturekind, data = tostring(tex) or error("not a string?") }
end

local aliases = { Color = "RGBSpectrum", RGB = "RGBSpectrum", blockbody = "BlockbodySpectrum", XYZ = "XYZSpectrum", Integer = "Int" }
for k,v in pairs(aliases) do
	_G[k] = _G[v]
end 